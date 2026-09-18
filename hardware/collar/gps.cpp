#include "gps.h"

#ifdef SIMULATION
    #include "../tools.h"
#else
    #include <Arduino.h>
    #include "hardware/gps/HT_TinyGPS++.h"
#endif

static TinyGPSPlus gGPS;

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
}

void GPS::onUpdate()
{
    while( Serial1.available() )
    {
        gGPS.encode(Serial1.read());
    }

    mIsReady = gGPS.location.isUpdated();


    if( mIsReady )
    {
        Serial.print("LAT: ");
        Serial.println(gGPS.location.lat(), 6);

        Serial.print("LON: ");
        Serial.println(gGPS.location.lng(), 6);

        Serial.print("SAT: ");
        Serial.println(gGPS.satellites.value());

        Serial.print("ALT: ");
        Serial.println(gGPS.altitude.meters());
    }
}
