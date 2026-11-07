#include "gps.h"

#include "../arduino.h"
#include "defines.h"


TinyGPSPlus gGPS;

#define EMULATE_GPS 1


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
#else
    // Enable GNSS power
    powerOn();
#endif

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
    }
#endif
}

void GPS::sleep() {

    DBG("GPS going to sleep..");

#ifndef EMULATE_GPS
    powerOff();
#endif
}

void GPS::powerOn()
{
    DBG("GPS power ON");

    // Release pin locks
    gpio_hold_dis((gpio_num_t)VGNSS_CTRL);
    gpio_hold_dis((gpio_num_t)GNSS_RST);
    gpio_hold_dis((gpio_num_t)GNSS_WAKE);

    // Power ON the white GNSS port
    pinMode(VGNSS_CTRL, OUTPUT);
    digitalWrite(VGNSS_CTRL, LOW);

    pinMode(GNSS_RST, OUTPUT);
    digitalWrite(GNSS_RST, HIGH);
    pinMode(GNSS_WAKE, OUTPUT);
    digitalWrite(GNSS_WAKE, HIGH);

    Serial1.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    DBG("GPS On");
    mIsPowered = true;
}


void GPS::powerOff()
{
    DBG("GPS power OFF");

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


