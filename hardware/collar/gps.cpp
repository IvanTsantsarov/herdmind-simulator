#include "gps.h"

#ifdef SIMULATION
    #include "../tools.h"
#else
    #include <Arduino.h>
    #include <TinyGPSPlus.h>
#endif

TinyGPSPlus gGPS;

#define VGNSS_CTRL 34
#define GPS_RX     39
#define GPS_TX     38


GPS::GPS() {}

void GPS::setup()
{
    // Enable GNSS power
    pinMode(VGNSS_CTRL, OUTPUT);
    digitalWrite(VGNSS_CTRL, LOW);

    // L76K UART
    Serial1.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    Serial.println("GPS started");

#ifdef SIMULATION
    gGPS.setupSimulation();
#endif
}


void GPS::onUpdate()
{
    while( Serial1.available() ){
        gGPS.encode(Serial1.read());
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
}
