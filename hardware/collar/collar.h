#ifndef COLLAR_H
#define COLLAR_H

#include <cstdint>

static uint32_t COLLAR_ID = 1000;

class Collar
{
    struct Package {

#ifdef SIMULATION
        // in simulation collar id is generated here
        uint32_t id = COLLAR_ID++;
#else
        // in real mode id should be overwritten in defines.h by the programming script
        uint32_t id = COLLAR_ID;
#endif
    };

    Package mPackage;

public:
    Collar();
private:
    void onDataReceived(const char *data, int length);
};

#endif // COLLAR_H
