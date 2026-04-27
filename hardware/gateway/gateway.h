#ifndef GATEWAY_H
#define GATEWAY_H


#include <QDateTime>
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

class Network;
class Mqtt;

#ifdef  SIMULATION
    #include <QPointF>

class Gateway : public QObject {

    Q_OBJECT

    QString mId;
    Mqtt* mClient = nullptr;

    QTimer mPullTimer;

    Network* mNetwork = nullptr;
    quint16 mFPort = 0;

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

    void startHeartbeat();

public:

    Gateway(const QSettings &settings);
    virtual ~Gateway();
    inline bool hasSim(){ return mHasSIM; }
    void process();
    bool publishOnline();

#ifdef  SIMULATION
    QPointF mPos;

    void start();
    void subscribe(const QString &topic);
    bool publish(const QByteArray &phyPayload);

    void onUpdate();
    void onSend();

    inline bool isSending(){ return mIsSending; }

    inline const QString& id(){ return mId; }

signals:
    void downlinkReceived(const QByteArray& response);

protected slots:
    void onStopSending();
    void onMessageReceived(const QByteArray &message, const QString &topicName);
#endif

};

#endif // GATEWAY_H
