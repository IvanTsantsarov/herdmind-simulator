#ifndef GATEWAY_H
#define GATEWAY_H

#include <QSettings>
#include <QTimer>
#include <QObject>
#include <QUdpSocket>
#include <cstdint>
#include <map>

// interval for reading the sensors
#define GATEWAY_UPDATE_INTERVAL 5000

// interval for sending data to collars/gateways
#define GATEWAY_SEND_INTERVAL 20000


#ifdef  SIMULATION
    #include <QPointF>

class Gateway : public QObject {

    Q_OBJECT

    QUdpSocket mSocket;
    QString mChirpIP;
    quint16 mChirpPort;

    QTimer mPullTimer;

    void sendPullData();

#else
class Gateway
{
#endif

    uint32_t mHeardId = 0;
    uint16_t mAnimalsCount = 0;

    std::map<uint32_t, uint32_t> mBolusesMap; // id_bolus, id_animal
    std::map<uint32_t, uint32_t> mCollarsMap; // id_collar, id_animal

    bool mHasSIM = false;
    bool mIsSending = false;

public:
    enum OutSubPackageType {
        Empty = 0,
        Bolus = 1,
        CollarSmall = 2,
        CollarBig = 3,
        BolusAndCollarSmall = 4,
        BolusAndCollarBig = 5
    };

    struct OutSubPackageBolus {
        uint8_t battery;  // battery charge
        uint8_t status;
        int16_t temperature;
    };

    struct OutSubPackageCollarSmall {
        uint8_t battery;  // battery charge
        int16_t lonDelta; // longitude delta from the cented of the herd
        int16_t latDelta; // latitude delta from the cented of the herd
    };

    struct OutSubPackageCollarBig {
        uint8_t battery;  // battery charge
        uint32_t longitude; // full position longitue
        uint32_t latitude; // full position longitue
    };

    struct OutPackageHerd {
        uint32_t herd_id;  // herd id
        uint32_t ts;       // time stamp
        int32_t longitude; // herd longitude
        int32_t latitude;  // herd latitude
        uint16_t data_sz;  // folowing data  size
    };

    enum InPackageType {
        Init = 0,
        Action = 1
    };

    struct InPackageInit {
        uint32_t herd_id;
        uint16_t count = 0; // animals count
    };

    struct InPackageAnimal {
        uint32_t animal_id = 0;
        uint32_t bolus_id  = 0;
        uint32_t collar_id = 0;
    };

    struct InPackageAction {
        uint32_t request_id = 0;
        uint32_t animal_id = 0;
        uint8_t sound = 0;
        uint8_t light = 0;
    };


public:

    Gateway(const QSettings &settings);
    inline bool hasSim(){ return mHasSIM; }
    void process();

#ifdef  SIMULATION
    QPointF mPos;

    void onUpdate();
    void onSend();
    void start();

    bool sendToChirpStack(const QByteArray &phyPayload);
    inline bool isSending(){ return mIsSending; }
signals:
    void downlinkReceived(const QByteArray& response);
protected slots:
    void onUdpReadyRead();
    void onStopSending();



#endif

};

#endif // GATEWAY_H
