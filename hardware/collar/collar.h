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
    enum SoundId {
        beep1 = 1,
        beep2 = 2
    };

    enum LightId {
        once = 1,
        twice = 2
    };

public:
    // 18 bytes
    struct PackageOut {
        uint32_t id;        // collar id
        uint16_t seq;       // sequence
        int32_t longitude;  // positioning longitude
        int32_t latitude;   // positioning latitude
        uint8_t battery;    // battery level
        uint8_t event;      // event type level
        uint16_t count;     // boluses count (comming after this package)
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
    quint32 mAddr = 0;
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

    inline quint32 addr() const { return mAddr; }

    PackageOut getPackageOut(){ return mPackage; };
    QList<PackageBolusOut> getBoluses();
#endif


};

struct CollarData {
    Collar::PackageOut mCollar;
    QList<Collar::PackageBolusOut> mBoluses;
};


#endif // COLLAR_H
