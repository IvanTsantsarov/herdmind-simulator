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

    // 18 bytes
    struct PackageOut {
        uint32_t id = 0;        // collar id
        uint16_t seq = 0;       // sequence
        uint32_t longitude = 0;  // positioning longitude
        uint32_t latitude = 0;   // positioning latitude
        uint8_t battery = 0;    // battery level
        uint8_t event = 0;      // event type level
        uint16_t count = 0;     // boluses count (comming after this package)
    };

    // 8 bytes
    struct PackageBolusOut {
        uint32_t bolus_id;  // bolus id
        uint16_t seq;       // sequence
        uint8_t condition;  // see bolus.h
        uint8_t battery;    // battery level
    };

    struct PackageIn {
        uint32_t request_id = 0;
        uint8_t sound_id = 0;
        uint8_t light_id = 0;
    };

private:
#ifdef SIMULATION
    Animal* mAnimal;
    void onUpdate();
    void onSend();
#else
    Collar();
#endif
    PackageOut mPackage;

public:

#ifdef SIMULATION

    Collar(Animal* animal,
            const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray() );

    PackageOut getPackageOut(){ return mPackage; };
    QList<PackageBolusOut> getBoluses();
#endif


};

struct CollarData {
    Collar::PackageOut mCollar;
    QList<Collar::PackageBolusOut> mBoluses;
};


#endif // COLLAR_H
