#include "gps.h"

#include "../arduino.h"
#include "defines.h"
#include "../protocol.h"


TinyGPSPlus gGPS;


RTC_DATA_ATTR float GPS::mFenceMaxDistance = 0;
RTC_DATA_ATTR int GPS::mTrajectoryPointsCount = 0;
RTC_DATA_ATTR GeoPoint GPS::mTrajectoryPoints[TRAJECTORY_MAX_POINTS];
RTC_DATA_ATTR GeoPoint GPS::mLastGeoPos;

RTC_DATA_ATTR GeoPoint GPS::mGeoCenter;
RTC_DATA_ATTR int GPS::mFencePointsCount = 0;
RTC_DATA_ATTR GeoPoint GPS::mFenceGeoPoints[VIRTUAL_FENCE_MAX_POINTS];


// #define EMULATE_GPS 1


#define VGNSS_CTRL 34
#define GPS_RX     39
#define GPS_TX     38
#define GNSS_RST   42
#define GNSS_WAKE  40  // Wakeup pin


GPS::GPS() {}

void GPS::setup(bool isWakeup)
{
#ifdef EMULATE_GPS
    DBG("*** GPS emulation is ON ***");
    return;
#endif

    // 1. Release the deep sleep hold lock so the CPU can control the pin again
    gpio_hold_dis((gpio_num_t)VGNSS_CTRL);
    gpio_deep_sleep_hold_dis();

    pinMode(VGNSS_CTRL, OUTPUT);
    digitalWrite(VGNSS_CTRL, LOW);

    Serial1.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

#ifdef ONPC
    gGPS.setupSimulation();
#endif

}


void GPS::onUpdate()
{
#ifdef EMULATE_GPS
    mIsConnection = true;
    mIsReady = true;
    mPos.mLat = 42.1392059326f;
    mPos.mLon = 24.7586383820;
    mPos.mAlt = 150;
    mSatelites = 5;
#else
    while( Serial1.available() ){
        gGPS.encode(Serial1.read());
        mIsHardware = true;
    }

    if( !gGPS.location.isValid() ) {
        mIsConnection = false;
        //Serial.println("GPS is invalid");
        //Serial.println( String("GPS satelites: ") + gGPS.satellites.value());
        return;
    }

    mIsConnection = true;

    mIsReady = gGPS.location.isUpdated();

    if( mIsReady )
    {
        mPos.mLat = gGPS.location.lat();
        mPos.mLon = gGPS.location.lng();
        mPos.mAlt = gGPS.altitude.meters();
        mSatelites = gGPS.satellites.value();
        mLastPoint = Point::fromGeoPoint(mGeoCenter, mPos);
        updateTrajectory(mPos);
    }
#endif
}

void GPS::sleep()
{

    DBG("GPS going to sleep..");
#ifdef EMULATE_GPS
    return;
    // powerOff();
#endif

    // 1. Keep GNSS power ON (LOW) so L76K retains satellite ephemeris in RAM
    pinMode(VGNSS_CTRL, OUTPUT);
    digitalWrite(VGNSS_CTRL, LOW);

    // 2. Lock the pin state across deep sleep (ESP32-S3 requires BOTH calls)
    gpio_hold_en((gpio_num_t)VGNSS_CTRL);
    gpio_deep_sleep_hold_en();
}



void GPS::powerOff()
{
    // 1. Terminate the UART connection to release internal pullups/hardware blocks
    Serial1.end();

    // 2. Isolate UART pins to prevent current leakage through the traces
    pinMode(GPS_RX, INPUT);
    pinMode(GPS_TX, INPUT);

    // 3. Drive GNSS control lines low and lock them
    pinMode(GNSS_RST, OUTPUT);
    digitalWrite(GNSS_RST, LOW);
    gpio_hold_en((gpio_num_t)GNSS_RST);

    pinMode(GNSS_WAKE, OUTPUT);
    digitalWrite(GNSS_WAKE, LOW);
    gpio_hold_en((gpio_num_t)GNSS_WAKE);

    // 4. Cut power to the white GNSS port
    pinMode(VGNSS_CTRL, OUTPUT);
    digitalWrite(VGNSS_CTRL, HIGH);
    gpio_hold_en((gpio_num_t)VGNSS_CTRL);

    DBG("GPS Off");

    mIsPowered = false;
}


#ifdef ONPC
void GPS::inject(GeoPoint gp, int sat)
{
    gGPS.inject(gp.mLat, gp.mLon, gp.mAlt, sat);
}
#endif

bool GPS::isEmulated()
{
#ifdef EMULATE_GPS
    return true;
#else
    return false;
#endif
}

GeoPoint GPS::tajectoryPoint(int index) {
    assert(index >= 0 && index < mTrajectoryPointsCount);
    return mTrajectoryPoints[index];
}


void GPS::updateTrajectory(const GeoPoint& geoPt)
{
    // increment trajectory buffer counter
    mTrajectoryPointsCount ++;

    // if trajectory point counter exceeds maximum count
    // shift left the whole array with one element
    if( mTrajectoryPointsCount >= TRAJECTORY_MAX_POINTS ) {
        for( auto i = 1; i < TRAJECTORY_MAX_POINTS; i++) {
            mTrajectoryPoints[i-1] = mTrajectoryPoints[i];
        }
        mTrajectoryPointsCount = TRAJECTORY_MAX_POINTS;
    }

    mLastGeoPos = geoPt;

    // add current geo location to the end of the tragectory points buffer
    mTrajectoryPoints[mTrajectoryPointsCount-1] = geoPt;
    if( mTrajectoryPointsCount < 2 ) {
        return;
    }
}


// Returns true on fence on and false of fence off
bool GPS::setupFence(uint8_t count, const GeoPoint& center, const uint8_t *offsetsPtr)
{
    mFencePointsCount = count;
    if( !mFencePointsCount) {
        // sendEvent(Protocol::Collar::Event::FenceOff, 0);
        return false;
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

    // sendEvent(Protocol::Collar::Event::FenceOn, mFencePointsCount);
    return true;
}


bool GPS::isInsideFence()
{
    bool isInside = false;

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
            isInside = true;
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
            isInside = !isInside;
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

    return isInside;
}

