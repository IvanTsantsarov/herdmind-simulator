#include "collar.h"
#include "defines.h"
#include "battery.h"
#include "res.h"
#include "screen.h"
#include "gps.h"
#include "button.h"
#include "led.h"
#include "serialcmd.h"
#include "memory.h"

// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 100

#define COLLAR_DBG_INTERVAL 2000

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 100

#define SCREEN_AWAKE_MSEC_MAX 5000
#define SCREEN_LOADING_MSEC 2000

#define PIN_BTN_MAIN 0
#define PIN_LED_MAIN 35

#define SERIAL_TX_BUFFER_SIZE 1024

#ifdef ONPC

#include "../../animal.h"
#include "../tools.h"

//////////////////////////////////////////////////////////////
/// On PC
//////////////////////////////////////////////////////////////
Collar::Collar(Animal* animal,
               const QByteArray &devEUI,
               const QByteArray& appKey, const QByteArray &nwkKey)
    : LoraDevSim(QString("%1 collar").arg(animal->name()), LoraDevSim::Profile::Collar,
              COLLAR_UPDATE_INTERVAL, COLLAR_SEND_INTERVAL,
              devEUI, appKey, nwkKey), mAnimal(animal)
{
    commonConstructor();

    // TODO: this should not happened here, but must be send from chirpstack
    mAnimalName = SimTools::translateCyrilic( animal->name() );
    mIsMale = animal->isMale();
}

Collar::Collar(QString animalName, bool isMale,
               const QByteArray &devEUI,
               const QByteArray& appKey , const QByteArray &nwkKey)
: LoraDevSim(QString("%1 collar").arg(animalName), LoraDevSim::Profile::Collar,
                   COLLAR_UPDATE_INTERVAL, COLLAR_SEND_INTERVAL,
                 devEUI, appKey, nwkKey), mAnimalName(animalName), mIsMale(isMale)
{
    commonConstructor();
}


Protocol::Collar Collar::getPackageOut(){ return mPackage; }

QLine Collar::fenceClosestBorder() {
    if( !mFenceClosestBorder ) {
        return QLine();
    }

    return QLine(mFenceClosestBorder->begin().mX,
                 mFenceClosestBorder->begin().mY,
                 mFenceClosestBorder->end().mX,
                 mFenceClosestBorder->end().mY);
}

QPointF Collar::fenceClosestPoint() {
    if( !mFenceClosestBorder ) {
        return QPointF();
    }

    return QPointF(mFenceClosestPoint.mX, mFenceClosestPoint.mY);
}

const Animal *Collar::animal() const { return mAnimal; }

void Collar::sendToSerial(const char *str)
{
    mSerialCmd->writeToSerial(str);
}

QByteArray Collar::readFromSerial()
{
    return mSerialCmd->readFromSerial();
}

void Collar::inject(GeoPoint _pos, int _sat, int _rssi, int _snr, int _bat) {
    mGPS->inject(_pos, _sat), mInjectedRSSI = _rssi, mInjectedSNR = _snr, mInjectedBat = _bat;
}

#else
////////// REAL COLLAR

Collar* gCollar = nullptr;

Collar::Collar()
{
    // Only one instance of the class is permitted
    assert(nullptr == gCollar);
    gCollar = this;
    commonConstructor();
}
#endif

void Collar::commonConstructor()
{
    mScreen = new Screen(this);
    mGPS = new GPS;
    mBtnMain = new Button(PIN_BTN_MAIN);
    mLed = new Led;
    mSerialCmd = new SerialCmd(this);
    mBattery = new Battery;
    mMemory = new Memory;
}


Collar::~Collar()
{
    delete mScreen;
    delete mGPS;
    delete mBtnMain;
    delete mLed;
    delete mSerialCmd;
    delete mBattery;
    delete mMemory;
}

#ifndef ONPC
bool gMainButtonDown = false;
void gMainButtonInterrupt() {
    gMainButtonDown = true;
}

void Collar::sendDbg()
{
    if( !mIsDbgInfo ) {
        return;
    }

    String dbgStr = String(SERIAL_CMD_BEGIN) + "dbg:gps=" + satellites() + SERIAL_CMD_COMMA + gpsStr()
                    + "|rssi=" + rssi()
                    + "|snr=" + snr()
                    + "|bat=" + batteryInfo()
                    + "|btn=" + (gMainButtonDown ? "y":"n");

    Serial.println(dbgStr);
}

#endif






void Collar::onSetup()
{
    mStage = Stage::Setup;

    // allocate TX buffer for non-blocking sends
    Serial.setTxBufferSize(SERIAL_TX_BUFFER_SIZE);

    Serial.begin(SERIAL_BAUDRATE);
    delay(100);
    Serial.println("Setup collar...");

    mScreen->setup();

    mGPS->setup();

    mBtnMain->setup();

    mLed->setup(PIN_LED_MAIN);

    mBattery->setup();

#ifndef ONPC
    attachInterrupt(
        digitalPinToInterrupt(mBtnMain->pin()),
        gMainButtonInterrupt,
        FALLING
        );
    LoraDevHW::onSetup();
#else
    LoraDevSim::onSetup();
#endif

    mSetupMillis = millis();

    Serial.println("Setup collar finished.");

    mStage = Stage::Init;
}

void Collar::sleep()
{
#ifdef ONPC
    mStage = Stage::Sleep;
#else
    esp_sleep_enable_ext1_wakeup(
        1ULL << PIN_BTN_MAIN,
        ESP_EXT1_WAKEUP_ANY_LOW
        );

    esp_deep_sleep_start();
#endif
}


#ifdef ONPC
int Collar::rssi()
{
    return mInjectedRSSI;
}

int Collar::snr()
{
    return mInjectedSNR;
}
#else
int Collar::rssi()
{
    return -30;
}

int Collar::snr()
{
    return -80;
}
#endif


GeoPoint Collar::gps()
{
    if( !mGPS) {
        return GeoPoint();
    }

    return mGPS->pos();
}

String Collar::gpsStr()
{
    GeoPoint pos = gps();
    String lat (pos.mLat, 10);
    String lon (pos.mLon, 10);
    return lat + "," + lon;

}

int Collar::satellites()
{
    if( !mGPS) {
        return -1;
    }

    return mGPS->satelites();
}


int Collar::signalStrength()
{
    // 1. Calculate Estimated Signal Power (ESP)
    // ESP = RSSI - 10 * log10(1 + 10^(-SNR/10))
    float esp = rssi() - 10.0 * std::log10(1.0 + std::pow(10.0, -snr() / 10.0));

    // 2. Define functional hardware boundaries
    const float ESP_MAX = -30.0;  // Perfect signal (100%)
    const float ESP_MIN = -140.0; // Absolute noise/sensitivity floor (0%)

    // 3. Linear interpolation to find percentage
    float percent = ((esp - ESP_MIN) / (ESP_MAX - ESP_MIN)) * 100.0;

    // 4. Clamp the output strictly between 0 and 100
    int finalPercent = static_cast<int>(std::round(percent));
    return std::clamp(finalPercent, 0, 100);
}

String Collar::batteryInfo()
{
    return String( String((mBattery->isPresent() ? "yes" : "no"))
            + "," + mBattery->level())
           + "%," + mBattery->voltage()
           + "V";
}

bool Collar::flash()
{
    if( !mMemory->writeKey("akey", akey()) ) {
        return false;
    }

    if( !mMemory->writeKey("nkey", nkey()) ) {
        return false;
    }

    return true;
}

bool Collar::restore()
{
    uint8_t key[LORA_KEY_LEN] = {0};

    if( !mMemory->readKey("akey", key) ) {
        return false;
    }
    setAKey(key);

    if( !mMemory->readKey("nkey", key) ) {
        return false;
    }
    setNKey(key);

    return true;
}

void Collar::updateScreenLoading(bool isFlush)
{
    // Clear the internal buffer
    mScreen->clear();

    mScreen->drawArray(2, 2, 44, 44, vector_mono_44x44);
    // Draw static strings (X position, Y position, String)
    String v("Herdmind ");
    v += COLLAR_VERSION;
    mScreen->drawText(2, 2 + 44 + FONT_CY, v);

    // TODO: send animal name to the collar
    // mLib.drawStr(2, 2 + 44 + FONT_CY, mCollar->animalName().c_str());

    mScreen->drawText(44 + FONT_CX + 6, 32, "Loading..." );

    // Push the buffer contents to the physical screen hardware
    if( isFlush){
        mScreen->flush();
    }
}


void Collar::updateScreenNormal(bool isFlush)
{
    mScreen->clear();

    Screen::CenterH c = mScreen->drawTextTableCenterH( 1, mAnimalName, 6 );
    mScreen->drawArray( c.x2 + 2*FONT_CX, 4, 12, 12, mIsMale ? male_12x12 : female_12x12);

    // Draw satellite icon
    if( !satellites() ) {
        mScreen->drawArray( 2, 24, 24, 24, satellite_no_24x24);
    }else {

        mScreen->drawArray( 2, 24, 24, 24, satellite_24x24);

        GeoPoint pos = readGPS();
        // printGPS();

        // Draw GPS position
        String lat (pos.mLat, 10);
        String lon (pos.mLon, 10);
        mScreen->drawTextTable( 1, 3, lat, 24, 2 );
        mScreen->drawTextTable( 1, 4, lon, 24, 2 );
    }

    // Draw Lorawan icon
    mScreen->drawArray( 2, 48, 12, 12, lora_12x12);
    String tower(String(signalStrength()) + "%");
    mScreen->drawTextTable( 3, 5, tower, 0, 2 );


    if( mBattery->isPresent() ) {
        // Draw battery icon
        int batLevel20 = mBattery->level() / 20;

        uint8_t* iconArray = nullptr;
        switch(batLevel20) {
        case 0:  iconArray = battery_0_24x12; break;
        case 1: iconArray = battery_20_24x12; break;
        case 2: iconArray = battery_40_24x12; break;
        case 3: iconArray = battery_60_24x12; break;
        case 4: iconArray = battery_80_24x12; break;
        default: iconArray = battery_100_24x12; break;
        }

        mScreen->drawArray( 3*FONT_CX + 32, 4*FONT_CY + 1, 24, 12, iconArray );

        // Draw battery level
        String batstr = String(mBattery->level()) + "%";
        mScreen->drawTextTable( 13, 5, batstr, 0, 2 );
    }else {
        // Draw battery icon
        mScreen->drawArray( 3*FONT_CX + 32, 4*FONT_CY + 1, 24, 12, battery_no_24x12);
    }

    if( isFlush ) {
        mScreen->flush();
    }
}

void Collar::onMainBtn() {
    mBtnMain->update();
    if( mScreen->isSleeping() ) {
        if( !mIsSignalGPS) {
            updateScreenNormal(false);
        }
        mScreen->wakeup();
        mScreen->flush();
        Serial.println("Waking up screen...");
    }
    mAwakeningMillisScreen = millis();

    DBG("Main button press!");
}


void Collar::onUpdate()
{
    int64_t msec = millis();

#ifndef ONPC
    // after waking up
    if( gMainButtonDown ) {
        onMainBtn();
        sendDbg();
        gMainButtonDown = false;
        mDbgMsec = msec;
    }else
    if( COLLAR_DBG_INTERVAL < (msec - mDbgMsec) ) {
        sendDbg();
        mDbgMsec = msec;
    }
#endif

    if( Stage::Sleep == mStage) {
        return;
    }

    if( Stage::Init == mStage) {

        updateScreenLoading();

        restore();

        // Wait loading interval
        if( SCREEN_LOADING_MSEC < (msec - mSetupMillis)  ) {
            mStage = Stage::Operate;
            mAwakeningMillisScreen = msec;
        }
    }

    if( Stage::Operate != mStage ) {
        return;
    }

    mBattery->update();

    mSerialCmd->update();

    // Check screen if it's time to sleep
    uint32_t msecAwakenScreen = millis() - mAwakeningMillisScreen;
    if( !mScreen->isSleeping() && (msecAwakenScreen > SCREEN_AWAKE_MSEC_MAX) ) {

        mScreen->sleep();

#ifdef ONPC
        mScreen->flush();
#endif
    }

    mGPS->onUpdate();

    updateScreenNormal();

    if( mGPS->isConnection() )
    {
        if( mGPS->isReady() )
        {
            if( !mIsSignalGPS ) {
                DBG( String("GPS signal arrived in ") + msec + " msec" );
                mIsSignalGPS = true;
            }

            updateTrajectory(mGPS->pos());
            mLed->updateOn(1600, 1600);
        }else {
            // Serial.print("-");
            mLed->updateOn(300, 1000);
        }
    }else {
        // Serial.print(".");
        mLed->updateOn(100, 500);
    }

#ifndef ONPC
    delay(100);
#else

#endif
}

void Collar::onSend()
{
    Protocol::Collar package;

    package.mEvent = Protocol::Collar::Event::Package;
    GeoPoint coord = readGPS();
    package.encodeLat( coord.mLat );
    package.encodeLon( coord.mLon );
    package.mBattery = 100;

#ifdef ONPC
    sendPackage(package.toByteArray(), sizeof(Protocol::CollarByteArray));
#endif
}

void Collar::onReceive(uint8_t *data, uint32_t size)
{
    int offset = 0;
    const Protocol::Collar::Event event = static_cast<Protocol::Collar::Event>(data[offset++]);

    switch(event) {
    case Protocol::Collar::Event::SetupFence:
    {
        uint32_t count = data[offset]; offset += 1;
        double lat = Protocol::decodeLat( Protocol::readUint32(data, offset )); offset += sizeof(uint32_t);
        double lon = Protocol::decodeLon( Protocol::readUint32(data, offset)); offset += sizeof(uint32_t);
        onSetupFence( count, GeoPoint(lat, lon), data + offset );
    }
        break;
    case Protocol::Collar::Event::Light: break;
    case Protocol::Collar::Event::Sound: break;
    case Protocol::Collar::Event::Shock: break;
    // default: assert(0);
    case Protocol::Collar::Event::None:
    case Protocol::Collar::Event::Package:
    case Protocol::Collar::Event::FenceOn:
    case Protocol::Collar::Event::FenceOff:
        break;
    }
}

void Collar::restart()
{
    Serial.println("Restarting...");
    Serial.flush();
    delay(500);

#ifdef ONPC
#else
    ESP.restart();
#endif
}

void Collar::sendEvent(Protocol::Collar::Event event, uint32_t value)
{
    uint8_t buffer[1 + sizeof(uint32_t)];
    buffer[0] = static_cast<uint8_t>(event);
    Protocol::writeUint32(value, buffer, 1);
#ifdef ONPC
    sendPackage(buffer, sizeof(buffer));
#else

#endif
}



