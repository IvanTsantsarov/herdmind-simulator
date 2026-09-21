#include "collar.h"
#include "res.h"
#include "screen.h"
#include "gps.h"
#include "button.h"
#include "led.h"

#define SCREEN_AWAKE_MSEC_MAX 5000

#define PIN_BTN_MAIN 0
#define PIN_LED_MAIN 35

#ifndef SIMULATION
    Collar* gCollar = nullptr;
#endif


void Collar::createObjects()
{
    mScreen = new Screen(this);
    mGPS = new GPS;
    mBtnMain = new Button(PIN_BTN_MAIN);
    mLed = new Led;
}




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

Collar::Collar()
{
    // Only one instance of the class is permitted
    assert(nullptr == gCollar);
    gCollar = this;
    createObjects();
}
#endif

Collar::~Collar()
{
    delete mScreen;
}


bool Collar::isFence(){ return mFencePointsCount > 0; }

bool Collar::isInsideFence(){ return mIsInsideFence; }

bool Collar::isGoingAwayFromFence(){ return mFenceIsGoingAway; }

double Collar::fanceDistance(){ return mFenceDistance; }

bool Collar::hasClosestFenceBorder(){ return nullptr != mFenceClosestBorder ; }

Screen* Collar::screen()
{
    return mScreen;
}

Led *Collar::led()
{
    return mLed;
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


GeoPoint Collar::readGPS()
{
#ifdef SIMULATION
    QGeoCoordinate geoCoor = mAnimal->geoPos();
    return GeoPoint( geoCoor.latitude(), geoCoor.longitude());
#else
    return mGPS->pos();
#endif
}

#ifndef SIMULATION
void gMainButtonInterrupt() {
    if( !gCollar ) {
        return;
    }
    gCollar->onMainBtn();
}
#endif

void Collar::onMainBtn() {
    mBtnMain->update();
    if( mScreen->isSleeping() ) {
        mScreen->wakeup();
    }
    mAwakeningMillisScreen = millis();

    Serial.println("Main button press!");
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

void Collar::updateTrajectory(GeoPoint& geoPt)
{
    // increment trajectory buffer counter
    mTrajectoryPointsCount ++;

    // if trajectory point counter exceeds maximum count
    // shift left the whole array with one element
    if( mTrajectoryPointsCount >= COLLAR_MAX_GPS_POINTS) {
        for( auto i = 1; i < COLLAR_MAX_GPS_POINTS; i++) {
            mTrajectoryPoints[i-1] = mTrajectoryPoints[i];
        }
        mTrajectoryPointsCount = COLLAR_MAX_GPS_POINTS;
    }

    mLastGeoPos = geoPt;
    mLastPoint = Point::fromGeoPoint(mGeoCenter, mLastGeoPos);
    if( isFence()) {
        testFence();
    }

    // add current geo location to the end of the tragectory points buffer
    mTrajectoryPoints[mTrajectoryPointsCount-1] = geoPt;
    if( mTrajectoryPointsCount < 2 ) {
        return;
    }

}

void Collar::updateGPS()
{
    GeoPoint pos = readGPS();

    Serial.print("Pos: ");
    Serial.print(pos.mLat, 6);
    Serial.print(", ");
    Serial.print(pos.mLon, 6);
    Serial.print(", ");
    Serial.println(pos.mAlt, 3);
    Serial.print("SAT: ");
    Serial.println(mGPS->satelites());

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

    mScreen->flush();

    // update trajectory
    updateTrajectory(pos);
}


void Collar::onUpdate()
{
    uint32_t msec = millis();

    if( Stage::Sleep == mStage) {
        return;
    }

    if( Stage::Init == mStage) {
        Serial.println("Initializing collar...");
        mScreen->init();
        mStage = Stage::Operate;
        mAwakeningMillisScreen = msec;
    }

    if( Stage::Operate != mStage ) {
        return;
    }

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
            updateGPS();
            if( mLed->update() ) mLed->on(1600, 1600);

        }else {
            Serial.println("GPS not ready!");
            if( mLed->update() ) mLed->on(300, 1000);
        }
    }else {
        Serial.println("GPS not connection!");
        if( mLed->update() ) mLed->on(100, 500);
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

void Collar::onSetupFence(uint8_t count, const GeoPoint& center, const uint8_t *offsetsPtr)
{
    mFencePointsCount = count;
    if( !mFencePointsCount) {
        sendEvent(Protocol::Collar::Event::FenceOff, 0);
        return;
    }

    mGeoCenter = center;
    int coordsIndex = 0;
    for( uint8_t ptIndex = 0; ptIndex < count; ptIndex ++, coordsIndex += 4 ) {
        int16_t offsetLat = Protocol::readInt16(offsetsPtr, coordsIndex);
        int16_t offsetLon = Protocol::readInt16(offsetsPtr, coordsIndex + 2);
        GeoPoint geoPt(
            Protocol::decodeCoordOffset(offsetLat, center.mLat ),
            Protocol::decodeCoordOffset(offsetLon, center.mLon ) );

        // qInfo() << QString::number(geoPt.mLat, 'f', 6) << QString::number(geoPt.mLon, 'f', 6); // trash
        mFenceGeoPoints[ptIndex] = geoPt;
        mFencePoints[ptIndex] = Point::fromGeoPoint(center, geoPt);
    }

    for( auto bIndex = 0; bIndex < mFencePointsCount; bIndex ++ ) {
        int nextIndex = bIndex + 1;
        if( nextIndex >= mFencePointsCount) {
            nextIndex = 0;
        }
        mFenceBorders[bIndex] = Border(mFencePoints[bIndex], mFencePoints[nextIndex]);
    }

    sendEvent(Protocol::Collar::Event::FenceOn, mFencePointsCount);
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

void Collar::testFence()
{
    mIsInsideFence = false;

    Point p = mLastPoint;

    // Find closest border and poind
    double closestDistSq = MAXFLOAT;
    int closestBorderIndex = -1;
    Point closestProj;

    // Test for inside/outside of the fence
    for( auto i = 0; i < mFencePointsCount; i ++) {
        Border& border = mFenceBorders[i];

        if( border.isOn(mLastPoint)) {
            closestDistSq = 0.0;
            mIsInsideFence = true;
            closestBorderIndex = i;
            closestProj = mLastPoint;
            break;
        }

        const Point& a = border.begin();
        const Point& b = border.end();

        bool intersects =
            ((a.mY > p.mY) != (b.mY > p.mY)) &&
            (p.mX < (b.mX - a.mX) * (p.mY - a.mY) /
                            (b.mY - a.mY) + a.mX);

        if (intersects) {
            mIsInsideFence = !mIsInsideFence;
        }

        // find closest side by the projection point
        Point proj = border.proj(p);
        double distSq = p.distSq(proj);
        if( distSq < closestDistSq && border.isInside(proj)) {
            closestProj = proj;
            closestDistSq = distSq;
            closestBorderIndex = i;
        }
    }

    // if no closest border found
    // reset search for closest end point
    // else try to find closest end point
    if( closestBorderIndex < 0 ) {
        closestDistSq = MAXFLOAT;
    }

    // Fist closest border end point then projection point
    int closestPointIndex = -1;
    for( auto i = 0; i < mFencePointsCount; i ++) {
        Point& bp = mFencePoints[i];
        double distSq = bp.distSq(p);
        if( distSq < closestDistSq ) {
            closestDistSq = distSq;
            closestPointIndex = i;
        }
    }

    // If closer end point of the border found,
    // then find which border is better visible
    // from the current animal point
    if( closestPointIndex >= 0 ) {
        int borderRightIndex = closestPointIndex;
        int borderLeftIndex = closestPointIndex - 1;
        if( borderLeftIndex < 0 ) {
            borderLeftIndex = mFencePointsCount - 1;
        }

        Border& borderRight = mFenceBorders[borderRightIndex];
        Border& borderLeft = mFenceBorders[borderLeftIndex];

        closestBorderIndex = borderLeft.isBetterVisible(borderRight, p) ?
                                 borderLeftIndex : borderRightIndex;
    }

    if(closestBorderIndex >= 0) {
        mFenceIsGoingAway = closestDistSq > mFenceClosestDistSq;
        mFenceClosestDistSq = closestDistSq;
        mFenceClosestBorder = &mFenceBorders[closestBorderIndex];
        mFenceClosestPoint = closestProj;
        mFenceDistance = std::sqrt(mFenceClosestDistSq);
    }else {
        assert(0);
    }
}
