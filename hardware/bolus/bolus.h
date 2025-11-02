#ifndef BOLUS_H
#define BOLUS_H

#include <cstdint>

// interval for reading the sensors
#define BOLUS_UPDATE_INTERVAL 2000

// interval for sending data to collars
#define BOLUS_SEND_INTERVAL 20000

#ifdef SIMULATION
#include <QObject>
#include <QTimer>
#include "../tools.h"

class Animal;
class Accel;
static uint32_t BOLUS_ID = 1000;

class Bolus : public QObject
{
    Q_OBJECT;
#else
class Bolus
{
#endif

#ifdef SIMULATION
    // in simulation bolus id is generated here
    uint32_t mId = BOLUS_ID++;
    #define BOLUS_SENDING_MSEC 2000
#else
    // in real mode id should be overwritten in defines.h by the programming script
    uint32_t mId;
#endif

public:

    enum Condition {
        Unavailable = 1,
        Unknown = 2,
        Atony = 4,
        Hypomotility = 8,
        Hyperactivity = 16
    };

    // 8 bytes
    struct Package {
        uint32_t id = 0;    // bolus id
        uint16_t s = 0;     // sequence
        int16_t t = 0;      // temperature
        uint8_t c = 0;      // condition flags - see Condition structure
        uint8_t b = 0;      // battery level
    };

private:
    Package mPackage;

    uint16_t mSequence = 0;
    uint8_t mCondition = 0;
    uint32_t mSendingDelay = 0;
    float mT = 0.0f, mAx  = 0.0f, mAy  = 0.0f, mAz  = 0.0f;

    inline void setCondition(Condition cond){ mCondition |= (cond << 1); }

    Accel* mAccel = nullptr;

#ifdef SIMULATION
    Animal* mAnimal = nullptr;
    QTimer mTimerUpdate;
    int mSendingMsec;
private slots:
    void onTimerStart();
    void onTimerUpdate();
#endif

public:

    Bolus();
    ~Bolus();

#ifdef SIMULATION
    Bolus(Animal* animal) : Bolus()
    {
        mAnimal = animal;

        QTimer delayTimer;
        delayTimer.singleShot( Tools::rnd(0, BOLUS_SEND_INTERVAL), this, &Bolus::onTimerStart );
    };

    void updateSendingMsec(int msec) {
        if( mSendingMsec <= 0 ) {
            return;
        }

        mSendingMsec -= msec;
    }

    bool isSendingData() {
        return mSendingMsec > 0;
    }
#endif

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
