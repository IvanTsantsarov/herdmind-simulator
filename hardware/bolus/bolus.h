#ifndef BOLUS_H
#define BOLUS_H

#include <cstdint>

// interval for reading the sensors
#define BOLUS_UPDATE_INTERVAL 5000

// interval for sending data to collars
#define BOLUS_SEND_INTERVAL 20000

#define HIGH_TEMPERATURE 39.8

#ifdef SIMULATION
#include <QObject>
#include <QTimer>
#include "../loradev.h"

class Animal;
class Accel;

class Bolus : public LoraDev {
    Animal* mAnimal = nullptr;
#else
class Bolus {}
#endif


public:

    enum Condition {
        Unavailable = 1,
        Unknown = 2,
        Atony = 4,
        Hypomotility = 8,
        Hyperactivity = 16,
        HighTemperature = 32
    };

    // 8 bytes
    struct Package {
        uint32_t id = 0;    // bolus id
        uint16_t s = 0;     // sequence
        uint8_t c = 0;      // condition flags - see Condition structure
        uint8_t b = 0;      // battery level
    };

private:
    Package mPackage;

    uint16_t mSequence = 0;
    uint8_t mCondition = 0;
    float mT = 0.0f, mAx  = 0.0f, mAy  = 0.0f, mAz  = 0.0f;

    inline void setCondition(Condition cond){ mCondition |= (cond << 1); }

    Accel* mAccel = nullptr;

    void init();

public:


#ifdef SIMULATION
    Bolus(Animal* animal);
    void onUpdate();
    void onSend();
#else
    Bolus();
#endif

    ~Bolus();

    bool readTemperature();

    bool readMotion();

    void preparePackage();

    void process();

    float t(){ return mT; }
    float ax(){ return mAx; }
    float ay(){ return mAy; }
    float az(){ return mAz; }
};

#endif // BOLUS_H
