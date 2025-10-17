#ifndef COLLAR_H
#define COLLAR_H

#include <cstdint>

static uint32_t COLLAR_ID = 1000;

#ifdef SIMULATION
class Animal;
#endif


class Collar
{

#ifdef SIMULATION
    // in simulation collar id is generated here
    uint32_t mId = COLLAR_ID++;
#else
    // in real mode id should be overwritten in defines.h by the programming script
    uint32_t mId = COLLAR_ID;
#endif

    enum SoundId {
        beep1 = 1,
        beep2 = 2
    };

    enum LightId {
        once = 1,
        twice = 2
    };

public:
    // 15 bytes
    struct PackageOut {
        uint32_t id;        // collar id
        int32_t longitude;  // positioning longitude
        int32_t latitude;   // positioning latitude
        uint8_t battery;    // battery level
        uint16_t count;     // read boluses count
    };

    struct PackageIn {
        uint32_t request_id = 0;
        uint8_t sound_id = 0;
        uint8_t light_id = 0;
    };

private:
#ifdef SIMULATION
    Animal* mAnimal;
#else
    Collar();
#endif

    PackageOut mPackage;

    void onDataReceived(const char *data, int length);
public:
    Collar();

#ifdef SIMULATION
    Collar(Animal* animal)
    {
        mAnimal = animal;
        Collar();
    }
#endif

    bool sendPackage();

};

#endif // COLLAR_H
