#ifndef COLLAR_H
#define COLLAR_H

#include <cstdint>

// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 5000

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 20000


struct CollarData;

#ifdef SIMULATION
#include <QPointF>
#include "../loradev.h"
#include "../protocol.h"


class Collar : public LoraDev

#else
class Collar
#endif
{

    uint16_t mSequence = 0;

public:
    typedef enum Event {
        Light = 1,
        Sound,
        Shock
    } EventType;

private:
#ifdef SIMULATION
    Animal* mAnimal;
    void onUpdate();
    void onSend();
#else
    Collar();
#endif
    Protocol::Collar mPackage;

public:

#ifdef SIMULATION

    Collar(Animal* animal,
            const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray() );

    Protocol::Collar getPackageOut(){ return mPackage; };
    QList<Protocol::Collar> getBoluses();
#endif


};

struct CollarData {
    Protocol::Collar mCollar;
    QList<Protocol::Bolus> mBoluses;
};


#endif // COLLAR_H
