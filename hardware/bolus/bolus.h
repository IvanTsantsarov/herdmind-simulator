#ifndef BOLUS_H
#define BOLUS_H

#include <cstdint>

static uint32_t BOLUS_ID = 1000;

class Bolus
{
    struct Package {

#ifdef SIMULATION
        // in simulation bolus id is generated here
        uint32_t id = BOLUS_ID++;
#else
        // in real mode id should be overwritten in defines.h by the programming script
        uint32_t id = BOLUS_ID;
#endif
        uint16_t seq;
        int16_t t = 0;
        int16_t ax = 0;
        int16_t ay = 0;
        int16_t az = 0;
    };

    Package mPackage;


    float mT = 0.0f, mAx  = 0.0f, mAy  = 0.0f, mAz  = 0.0f;

public:
    Bolus();

    bool readTemperature();

    bool readMotion();

    void preparePackage();

    bool sendPackage();

    void process();

    float t(){ return mT; }
    float ax(){ return mAx; }
    float ay(){ return mAy; }
    float az(){ return mAz; }
};

#endif // BOLUS_H
