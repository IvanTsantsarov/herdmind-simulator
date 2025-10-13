#ifndef COLLAR_H
#define COLLAR_H

#include <cstdint>

static uint32_t COLLAR_ID = 1000;

#ifdef SIMULATION
class Animal;
#endif


class Collar
{
public:
    struct Package {

#ifdef SIMULATION
        // in simulation collar id is generated here
        uint32_t id = COLLAR_ID++;
#else
        // in real mode id should be overwritten in defines.h by the programming script
        uint32_t id = COLLAR_ID;
#endif
    };

private:
#ifdef SIMULATION
    Animal* mAnimal;
#else
    Collar();
#endif

    Package mPackage;

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
