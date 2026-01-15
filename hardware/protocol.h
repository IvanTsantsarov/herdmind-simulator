#ifndef PROTOCOL_H
#define PROTOCOL_H


#include <cstdint>


#define VERSION 1
#define MAX_PACKAGE_SIZE (1024)


class Protocol
{
public:
    enum DevCont{
        // Chemical
        T = 0x10,    // Temperature
        pH = 0x20,
        Hum = 0x40,  // humidity
        CO2 = 0x80,  // Carbon dioxide

        // Phisycal
        Accel = 0x100, // Acceleration
        Orient = 0x200, // Orientation
        Pos = 0x400,     // Geo coordinates
        Compass = 0x800,
    };

    struct DevHead{
        uint8_t version = VERSION;
        uint32_t dev_id = 0;
        uint32_t tick = 0;
        uint8_t content = 0;
        uint8_t signal;
        uint8_t battery;

        inline bool hasChemical(){ return 0x0F & content; }
        inline bool hasT() { return DevCont::T & content; }
        inline bool haspH() { return DevCont::pH & content; }
        inline bool hasHum() { return DevCont::Hum & content; }
        inline bool hasC02() { return DevCont::pH & content; }

        inline bool hasPhysical(){ return 0xF0 & content; }
        inline bool hasAccel() { return DevCont::Accel & content; }
        inline bool hasOrient() { return DevCont::Orient & content; }
        inline bool hasPos() { return DevCont::Pos & content; }
        inline bool hasCompass() { return DevCont::Compass & content; }
    };


    enum ServerCont {
        // Commands
        Light = 0x1,
        Sound = 0x2,
        Impulse = 0x4,
        Vibration = 0x8,

        Time = 0x10,
        Map = 0x20,
        Test = 0x40,
        Prog = 0x80
    };

    struct ServerHead {
        uint8_t version = VERSION;
        uint32_t tick = 0;
        uint8_t content = 0;

        inline bool hasLight() { return ServerCont::Light & content; }
        inline bool hasSound() { return ServerCont::Sound & content; }
        inline bool hasImpulse() { return ServerCont::Impulse & content; }
        inline bool hasVibration() { return ServerCont::Vibration & content; }
    };

    enum Error {
        NoError = 0,
        SmallerThenHeader,
        TooBig,

    };


private:

    static int16_t f2i16(float v, float scale);

public:
    Protocol();
    Error parseDevice(void *data, uint32_t size, uint32_t& parsedBytes);

    Error parseServer(void *data, uint32_t size, uint32_t& parsedBytes);
};


#endif // PROTOCOL_H
