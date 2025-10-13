#ifndef BOLUS_H
#define BOLUS_H

#include <cstdint>

#define BOLUS_UPDATE_INTERVAL 2000

#ifdef SIMULATION
#include <QObject>
#include <QTimer>

class Animal;
static uint32_t BOLUS_ID = 1000;

class Bolus : public QObject
{
    Q_OBJECT;
#else
class Bolus
{
#endif

public:
    struct Package {

#ifdef SIMULATION
        // in simulation bolus id is generated here
        uint32_t id = BOLUS_ID++;
#else
        // in real mode id should be overwritten in defines.h by the programming script
        uint32_t id;
#endif
        uint16_t seq;
        int16_t t = 0;
        int16_t ax = 0;
        int16_t ay = 0;
        int16_t az = 0;
    };

private:
    Package mPackage;

    float mT = 0.0f, mAx  = 0.0f, mAy  = 0.0f, mAz  = 0.0f;

#ifdef SIMULATION
    Animal* mAnimal = nullptr;
    QTimer mTimer;
private slots:
    void onTimer();
#endif

public:

    Bolus() { };

#ifdef SIMULATION
    Bolus(Animal* animal)
    {
        connect( &mTimer, &QTimer::timeout, this, &Bolus::onTimer );
        mTimer.start(BOLUS_UPDATE_INTERVAL);
        mAnimal = animal;
        Bolus();
    };
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
