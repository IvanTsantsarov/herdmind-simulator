#include "collar.h"
#include "battery.h"
#include "res.h"
#include "screen.h"
#include "gps.h"
#include "button.h"
#include "led.h"
#include "memory.h"
#include "lights.h"
#include "sound.h"
#include "shock.h"
#include "timer.h"
#include "bluetooth.h"


#define COLLAR_MAIN_BTN_BLE_INTERVAL 3000


// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 100

#define COLLAR_DBG_INTERVAL 2000

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 100

#define SCREEN_AWAKE_MSEC_MAX 5000
#define SCREEN_LOADING_MSEC 2000

#define PIN_BTN_MAIN 0
#define PIN_LED_MAIN 35

#ifdef ONPC

#include "../../animal.h"
#include "../tools.h"

//////////////////////////////////////////////////////////////
/// On PC
//////////////////////////////////////////////////////////////
// Contructor called on simulation
Collar::Collar(Animal* animal, bool isSimulation,
               const QByteArray &devEUI,
               const QByteArray& appKey,
               const QByteArray &nwkKey)
    : LoraDevSim(isSimulation, QString("%1 collar").arg(animal->name()),
                LoraDevSim::Profile::Collar,
                COLLAR_UPDATE_INTERVAL, COLLAR_SEND_INTERVAL,
                devEUI, appKey, nwkKey), mAnimal(animal)
{
    commonConstructor();

    // TODO: this should not happened here, but must be send from chirpstack
    setAnimalName( SimTools::translateCyrilic(animal->name()).toLatin1().data() );
    setIsMale(animal->isMale());
}

// Contructor called when NO simulation
Collar::Collar(QString animalName, bool isMale,
               const QByteArray &devEUI,
               const QByteArray& appKey ,
               const QByteArray &nwkKey,
               const QByteArray &addrHex)
    : LoraDevSim(false, QString("%1 collar").arg(animalName),
                LoraDevSim::Profile::Collar,
                COLLAR_UPDATE_INTERVAL, COLLAR_SEND_INTERVAL,
                devEUI, appKey, nwkKey)
{
    setAnimalName( SimTools::translateCyrilic(animalName).toLatin1().data() );
    setIsMale(isMale);
    setAddrHex(addrHex);
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

void Collar::sendToSerial(const char *str)
{
    mSerialCmd->writeToSerial(str);
}

QByteArray Collar::readFromSerial()
{
    return mSerialCmd->readFromSerial();
}

void Collar::inject(GeoPoint _pos, int _sat, float _rssi, float _snr, int _bat)
{
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
    mTimer = new Timer();
    mScreen = new Screen(this);
    mGPS = new GPS;
    mBtnMain = new Button(PIN_BTN_MAIN);
    mLed = new Led;
    mSerialCmd = new SerialCmd(this);
    mBattery = new Battery;
    mMemory = new Memory;
    mLights = new Lights(this);
    mSound = new Sound(this);
    mShock = new Shock(this);
    mBle = new Ble();
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
    delete mLights;
    delete mSound;
    delete mShock;
    delete mBle;
}

void Collar::sendStats()
{
    if( !mIsStats ) {
        return;
    }


    String statsStr = String("gps=") + satellites() + SERIAL_CMD_COMMA + gpsStr()
                      + "|rssi=" + rssi()
                      + "|snr=" + snr()
                      + "|bat=" + batteryInfo();

    mSerialCmd->printCmd(SerialCmd::Cmd::Type::STATS, statsStr);
}

bool Collar::gotoBed()
{
    mState.set( State::Sleep, mSendIntervalMsec );
    if( !isDeepSleep() ) {
        DBG(String("Light sleep for:") + mSendIntervalMsec);
        mScreen->sleep();
        return true;
    }

    DBG(String("Deep sleep for:") + mSendIntervalMsec);

    mScreen->off();
    mGPS->sleep();
    prepareToSleep(mSendIntervalMsec, PIN_BTN_MAIN);
    mTimer->sleep(mSendIntervalMsec);
    return sleep();
}


#ifdef ONPC
float Collar::rssi()
{
    return mRxInfo.rssi();
}

float Collar::snr()
{
    return mRxInfo.snr();
}
#endif

char Collar::gpsStatus()
{
    if( mGPS->isEmulated() ) return 'e';
    return mGPS->isHardware() ? 'y' : 'n';
}



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
    float currentRssi = rssi();
    float currentSnr  = snr();

    // 1. Guard against invalid sensor readings (NaN or Inf)
    if (std::isnan(currentRssi) || std::isnan(currentSnr)) {
        return 0;
    }

    // 2. Calculate Estimated Signal Power (ESP) in dBm
    float esp;
    if (currentSnr < 0.0f) {
        // Below noise floor: Semtech standard approximation avoids power-sum distortion
        esp = currentRssi + currentSnr;
    } else {
        // Above noise floor: Exact linear power separation of Signal from (Signal + Noise)
        esp = currentRssi - 10.0f * std::log10f(1.0f + std::powf(10.0f, -currentSnr / 10.0f));
    }

    // 3. Define functional hardware boundaries (adjust ESP_MIN to your Spreading Factor)
    // Typical sensitivity limits (125 kHz BW): SF7 = -123 dBm, SF10 = -132 dBm, SF12 = -137 dBm
    const float ESP_MAX = -50.0f;  // Strong signal (100%)
    const float ESP_MIN = -120.0f; // Near sensitivity limit (0%)

    // 4. Linear interpolation to percentage
    float percent = ((esp - ESP_MIN) / (ESP_MAX - ESP_MIN)) * 100.0f;

    // 5. Clamp strictly between 0 and 100
    int finalPercent = static_cast<int>(std::lround(percent));
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

    if( !mMemory->writeAddrHex(addrHex()) ) {
        return false;
    }

    if( !mMemory->writeName(animalName()) ) {
        return false;
    }

    if( !mMemory->writeSex(isMale())) {
        return false;
    }

    if( !mMemory->writeFPort(fport())) {
        return false;
    }

    return true;
}

bool Collar::restore()
{
    if( !mEnableRestore ) {
        return true;
    }

    uint8_t key[LORA_KEY_LEN] = {0};

    if( !mMemory->readKey("akey", key) ) {
        return false;
    }
    setAKey(key);

    if( !mMemory->readKey("nkey", key) ) {
        return false;
    }
    setNKey(key);

    char addrBufferHex[LORA_ADDR_HEX_LEN + 1] = {0};
    if( !mMemory->readAddrHex(addrBufferHex) ) {
        return false;
    }

    setAddrHex(addrBufferHex);

    char animalName[LORA_NAME_MAX_LEN + 1] = {0};
    if( !mMemory->readName(animalName)) {
        return false;
    }

    setAnimalName(animalName);

    bool isMale;
    if( !mMemory->readSex(isMale)) {
        return false;
    }

    setIsMale(isMale);

    uint8_t fp = 0;
    if( !mMemory->readFPort(fp)) {
        return false;
    }

    setFPort(fp);

    return true;
}

void Collar::serialRespond(const String& msg)
{
    mSerialCmd->printResponce(msg);
}

void Collar::serialCmd(SerialCmd::Cmd::Type t, const String& str)
{
    mSerialCmd->printCmd(t, str);
}

void Collar::execSignal(SignalType t, int mode)
{
    switch(t) {
    case SignalType::Light: mLights->execute(); break;
    case SignalType::Sound: mSound->execute(); break;
    case SignalType::Shock: mShock->execute(); break;
    }
}

void Collar::setEpoch(uint64_t secs)
{
    mTimer->setEpoch(secs);
}

uint64_t Collar::epoch()
{
    return mTimer->epoch();
}

String Collar::dateTime()
{
    return mTimer->dateTimeStr();
}

////////////////////////////////////////////////////////////////////////
/// \brief Setup
///
////////////////////////////////////////////////////////////////////////
bool gMainButtonClicked = false;
int64_t gMainButtonMsec = 0;
void onMainBtnClicked(int64_t msecDown)
{
    gMainButtonMsec = msecDown;
    gMainButtonClicked = true;
}

void Collar::setup()
{
    mState.set( isWakingUp() ? State::SetupWake : State::SetupBoot );

    mSerialCmd->setup();

    mScreen->setup();

    mSound->setup();

    mGPS->setup(isWakingUp());

    mBtnMain->setup(onMainBtnClicked);

    mLed->setup(PIN_LED_MAIN);

    mBattery->setup();

    onSetup();

    mSerialCmd->printCmd( SerialCmd::Cmd::Type::DBG, mTimer->dateTimeStr() );
    mSerialCmd->printCmd( SerialCmd::Cmd::Type::TM, String(mTimer->epoch()) );

    if( isWakingUp() ) {
        // restore last sleeping interval
        mSendIntervalMsec = mTimer->sleepMsec();

        if( isWakeBtn() ) {
            DBG("Wake up from button. Waking up the screen..");
            mScreen->wakeup();
        }else {
            mScreen->sleep();
        }

        mState.set( State::UpdateMaster );
    }else {
        // Bring back all the saved data
        restore();
        mState.set( State::Init );
        mSound->setVolume(128);
        mSound->play("A4B4C4D4E4", 100);
    }
}


void Collar::updateScreenProduct(bool isFlush)
{
    // Clear the internal buffer
    mScreen->clear();

    mScreen->drawArray(2, 2, 44, 44, vector_mono_44x44);
    // Draw static strings (X position, Y position, String)
    String v("Herdmind");
    mScreen->drawText(2, 2 + 44 + FONT_CY, v);

    // TODO: send animal name to the collar
    // mLib.drawStr(2, 2 + 44 + FONT_CY, mCollar->animalName().c_str());
    int dx = 44 + FONT_CX + 6;
    int dy = FONT_CY;

    mScreen->drawText(dx, dy, animalName() ); dy += FONT_CY + 1;
    mScreen->drawText(dx, dy, "Build:" ); dy += FONT_CY + 1;
    mScreen->drawText(dx, dy, String(__TIME__) ); dy += FONT_CY + 1;
    mScreen->drawText(dx, dy, String(__DATE__) ); dy += FONT_CY + 1;


    // Push the buffer contents to the physical screen hardware
    if( isFlush){
        mScreen->flush();
    }

    DBG("Screen LOADING updated.");
}


void Collar::updateScreenInfo(bool isFlush)
{
    mScreen->clear();

    Screen::CenterH c = mScreen->drawTextTableCenterH( 1, animalName(), 6 );
    mScreen->drawArray( c.x2 + 2*FONT_CX, 4, 12, 12, isMale() ? male_12x12 : female_12x12);

    // Draw satellite icon
    if( !satellites() ) {
        String str("???");
        mScreen->drawArray( 2, 24, 24, 24, satellite_no_24x24);
        mScreen->drawTextTable( 1, 3, str, 24, 2 );
        mScreen->drawTextTable( 1, 4, str, 24, 2 );
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

    DBG("Screen NORMAL updated.");
}

void Collar::onMainBtn(int64_t msecPress) {

    if( mScreen->isSleeping() )
    {
        DBG("Screen waking up...");
        updateScreen(false);
    }else {
        updateScreen(true);
        DBG("Screen is not sleeping!");
    }

    mScreen->wakeup();

    if( mState.isSleep() ) {
        mState.setPrev();
    }

    if( COLLAR_MAIN_BTN_BLE_INTERVAL > msecPress) {
        DBG("Main button press short!");
        mSound->play("B5F6", 50); // plays chirp sound

    }else {
        DBG("Main button press long!");
        mSound->play("A5C5E5", 200); // plays chirp sound
        mBle->on( animalName() );
    }


}

////////////////////////////////////////////////////////////////////////
/// \brief Update
///
////////////////////////////////////////////////////////////////////////
///

void Collar::update()
{
    mState.tick();

    // after waking up
    if( gMainButtonClicked ) {
        onMainBtn(gMainButtonMsec);
        gMainButtonClicked = false;
    }

    if( !mScreen->isSleeping() && (mScreen->msecWake() > SCREEN_AWAKE_MSEC_MAX) ) {
        mScreen->sleep();
    }

    updateSerial();

    if( mState.isInit() ) {
        updateScreen();
        mState.set(State::UpdateMaster);
    }else
    if( mState.isSleep() ) {
        if( mState.isFinished() ) {
            mState.set(State::UpdateMaster);
        }else {
            delay(100);
            return;
        }
    }

    if( mState.isFirstTick() || mState.isNewSecond() ) {
        mBattery->update();
        mGPS->onUpdate();
        sendStats();
        updateMaster();
        mLed->blink(10);
    }else {
        delay(100);
        return;
    }

    // go to sleep if no satelite signal
    if( mState.isElapsedSec(mSecWakeInterval) && mScreen->isSleeping() ) {
        DBG(String("No GPS signal for ") + mSecWakeInterval + " sec. Preparing to sleep..");
        gotoBed();
    }

}


void Collar::updateMaster()
{
    if( mGPS->isConnection() )
    {
        if( mGPS->isReady() )
        {
            mLed->hwOn();

            updateScreen();

            if( !mIsSignalGPS ) {
                DBG( String("GPS signal arrived in ") + mState.elapsedSec() + " sec" );
                mIsSignalGPS = true;
            }

            updateTrajectory(mGPS->pos());

#ifdef ONPC
            // Do not send information when there is no simulation
            if( isSimulation() ) {
                onSend();
            }
#else
            onSend();
#endif
            // once again
            updateSerial();

            mLed->hwOff();
            gotoBed();
        }
    }
}



void Collar::updateSerial()
{
    mSerialCmd->update();

    // Wait 1 second for time sync if initiated
    int waitingCounter = 0;
    while( mSerialCmd->isWaitingForTimeSync() && waitingCounter < 100 ) {
        delay(10);
        mSerialCmd->update();
        waitingCounter ++;
    }

    if( mSerialCmd->isWaitingForTimeSync() ) {
        mSerialCmd->printError("Time sync failed!");
    }

    mSerialCmd->stopWaitingForTimeSync();
}

void Collar::updateScreen(bool isIncrement)
{
    switch(mScreenMode) {
    case ScreenMode::Product: updateScreenProduct(); break;
    case ScreenMode::Info: updateScreenInfo(); break;
    default: break;
    }

    if( isIncrement ) {
        incrementScreenMode();
    }
}


////////////////////////////////////////////////////////////////////////
/// \brief Send/Receive
///
////////////////////////////////////////////////////////////////////////


void Collar::onSend()
{
    Protocol::Collar package;

    // DBG("Collar::onSend()");

    package.mEvent = Protocol::Collar::Event::Package;
    GeoPoint coord = readGPS();
    package.encodeLat( coord.mLat );
    package.encodeLon( coord.mLon );
    package.mBattery = mBattery->level();
    package.mStatus = 0; // TODO

    sendPackage(package.toByteArray(), sizeof(Protocol::CollarByteArray));
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
    case Protocol::Collar::Event::Light:
        mLights->execute();
        break;
    case Protocol::Collar::Event::Sound:
        mSound->execute();
        break;
    case Protocol::Collar::Event::Shock:
        mShock->execute();
        break;
    // default: assert(0);
    case Protocol::Collar::Event::None:
    case Protocol::Collar::Event::Package:
    case Protocol::Collar::Event::FenceOn:
    case Protocol::Collar::Event::FenceOff:
        break;
    }
}

void Collar::reboot()
{
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



