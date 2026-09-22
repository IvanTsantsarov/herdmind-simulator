#include "collar.h"
#include "defines.h"
#include "res.h"
#include "screen.h"
#include "gps.h"
#include "button.h"
#include "led.h"

#define SCREEN_AWAKE_MSEC_MAX 5000

#define PIN_BTN_MAIN 0
#define PIN_LED_MAIN 35

#ifdef SIMULATION

#include "../../animal.h"
#include "../tools.h"

//////////////////////////////////////////////////////////////
/// Simulation
//////////////////////////////////////////////////////////////
Collar::Collar( Animal* animal,
               const QByteArray &devEUI,
               const QByteArray& appKey)
    : LoraDev(QString("%1 collar").arg(animal->name()), LoraDev::Profile::Collar,
              COLLAR_UPDATE_INTERVAL, COLLAR_SEND_INTERVAL,
              devEUI, appKey), mAnimal(animal)
{
    createObjects();

    // TODO: this should not happened here, but must be send from chirpstack
    mAnimalName = SimTools::translateCyrilic( animal->name() );
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

#else
////////// REAL COLLAR

Collar* gCollar = nullptr;

Collar::Collar()
{
    // Only one instance of the class is permitted
    assert(nullptr == gCollar);
    gCollar = this;
    createObjects();
}
#endif

void Collar::createObjects()
{
    mScreen = new Screen(this);
    mGPS = new GPS;
    mBtnMain = new Button(PIN_BTN_MAIN);
    mLed = new Led;
}


Collar::~Collar()
{
    delete mScreen;
    delete mGPS;
    delete mBtnMain;
    delete mLed;
}

#ifndef SIMULATION
bool gMainButtonDown = false;
void gMainButtonInterrupt() {
    gMainButtonDown = true;
}
#endif


void Collar::onMainBtn() {
    mBtnMain->update();
    if( mScreen->isSleeping() ) {
        if( !mIsSignal) {
            updateScreenLoading(false);
        }else {
            updateScreenNormal(false);
        }
        mScreen->wakeup();
        mScreen->flush();
    }
    mAwakeningMillisScreen = millis();

    DBG("Main button press!");
}


void Collar::onSetup()
{
    mStage = Stage::Setup;

    Serial.begin(SERIAL_BAUDRATE);
    delay(100);
    Serial.println("Setup collar...");

    mScreen->setup();

    mGPS->setup();

    mBtnMain->setup();

    mLed->setup(PIN_LED_MAIN);

#ifndef SIMULATION
    attachInterrupt(
        digitalPinToInterrupt(mBtnMain->pin()),
        gMainButtonInterrupt,
        FALLING
        );
#endif
    mStage = Stage::Init;

}

void Collar::sleep()
{
#ifdef SIMULATION
    mStage = Stage::Sleep;
#else
    esp_sleep_enable_ext1_wakeup(
        1ULL << PIN_BTN_MAIN,
        ESP_EXT1_WAKEUP_ANY_LOW
        );

    esp_deep_sleep_start();
#endif
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

#ifdef SIMULATION
    String animalName = SimTools::translateCyrilic( mAnimal->name() );
    bool isMale = mAnimal->isMale();
#else
    // TODO: set here name and gender
    String animalName("<Animal name>");
    bool isMale = false;
#endif
    Screen::CenterH c = mScreen->drawTextTableCenterH( 1, animalName, 6 );
    mScreen->drawArray( c.x2 + 2*FONT_CX, 4, 12, 12, isMale ? male_12x12 : female_12x12);

    // Draw satellite icon
    mScreen->drawArray( 2, 24, 24, 24, satellite_24x24);

    GeoPoint pos = readGPS();
    printGPS();

    // Draw GPS position
    String lat (pos.mLat, 10);
    String lon (pos.mLon, 10);
    mScreen->drawTextTable( 1, 3, lat, 24, 2 );
    mScreen->drawTextTable( 1, 4, lon, 24, 2 );

    // Draw Lorawan icon
    mScreen->drawArray( 2, 48, 12, 12, lora_12x12);
    String tower("35%");
    mScreen->drawTextTable( 3, 5, tower, 0, 2 );

    // Draw battery icon
    mScreen->drawArray( 3*FONT_CX + 32, 4*FONT_CY + 1, 24, 12, battery_60_24x12);

    // Draw battery level
    String battery("60%");
    mScreen->drawTextTable( 13, 5, battery, 0, 2 );

    if( isFlush ) {
        mScreen->flush();
    }
}


void Collar::onUpdate()
{
#ifndef SIMULATION
    // after waking up
    if( gMainButtonDown ) {
        onMainBtn();
        gMainButtonDown = false;
    }
#endif

    uint32_t msec = millis();

    if( Stage::Sleep == mStage) {
        return;
    }

    if( Stage::Init == mStage) {
        Serial.println("Initializing collar...");
        updateScreenLoading();
        mStage = Stage::Operate;
        mAwakeningMillisScreen = msec;
    }

    if( Stage::Operate != mStage ) {
        return;
    }

    // Check screen if it's time to sleep
    uint32_t msecAwakenScreen = msec - mAwakeningMillisScreen;
    if( msecAwakenScreen > SCREEN_AWAKE_MSEC_MAX ) {
        mScreen->sleep();
#ifdef SIMULATION
        mScreen->flush();
#endif
    }

    mGPS->onUpdate();

    if( mGPS->isConnection() )
    {
        if( mGPS->isReady() )
        {
            if( !mIsSignal ) {
                Serial.println( String("GPS signal arrived in") + millis() + " msec" );
                mIsSignal = true;
            }

            updateScreenNormal();
            updateTrajectory(mGPS->pos());
            mLed->updateOn(1600, 1600);
        }else {
            Serial.print("-");
            mLed->updateOn(300, 1000);
        }
    }else {
        Serial.print(".");
        mLed->updateOn(100, 500);
    }



#ifndef SIMULATION
    delay(100);
#else
    gTools.update(mAnimal);
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

#ifdef SIMULATION
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

void Collar::sendEvent(Protocol::Collar::Event event, uint32_t value)
{
    uint8_t buffer[1 + sizeof(uint32_t)];
    buffer[0] = static_cast<uint8_t>(event);
    Protocol::writeUint32(value, buffer, 1);
#ifdef SIMULATION
    sendPackage(buffer, sizeof(buffer));
#else

#endif
}

String &Collar::animalName() { return mAnimalName; }

