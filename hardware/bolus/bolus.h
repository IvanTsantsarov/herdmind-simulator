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
#include "../protocol.h"

class Animal;
class Accel;

class Bolus : public LoraDev {
    Animal* mAnimal = nullptr;
#else
class Bolus {}
#endif



private:
    Protocol::Bolus mPackage;

    uint16_t mSequence = 0;
    uint8_t mCondition = 0;
    float mT = 0.0f, mAx  = 0.0f, mAy  = 0.0f, mAz  = 0.0f;

    inline void setCondition(Protocol::Bolus::Condition cond){ mCondition |= (cond << 1); }

    Accel* mAccel = nullptr;

    void init();

public:


#ifdef SIMULATION
    Bolus(Animal* animal,
          const QByteArray &devEUI = QByteArray(),
          const QByteArray& appKey = QByteArray());

    void onUpdate();
    void onSend();
    void onReceive(uint8_t* data){};
#else
    Bolus();
#endif

    ~Bolus();

    bool readTemperature();

    bool readMotion();

    void process();

    float t(){ return mT; }
    float ax(){ return mAx; }
    float ay(){ return mAy; }
    float az(){ return mAz; }
};

#endif // BOLUS_H
