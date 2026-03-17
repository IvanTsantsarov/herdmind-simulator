#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "gateway.h"
#include "defines_settings.h"

#ifdef SIMULATION

Gateway::Gateway(const QSettings &settings) :
    mClient(this)
{
    mMqttAddr = settings.value(MQTT_SECTION"/address").toString();
    mMqttPort = settings.value(MQTT_SECTION"/port").toUInt();
    mId = settings.value(MQTT_SECTION"/gatewayId").toString();
    QString userName = settings.value(MQTT_SECTION"/username").toString();
    QString password = settings.value(MQTT_SECTION"/password").toString();

    // From docker-compose: mosquitto exposes 1883 to host
    mClient.setHostname(mMqttAddr);
    mClient.setPort(mMqttPort);

    if( !userName.isEmpty() ) {
        mClient.setUsername(userName);
    }

    if( !password.isEmpty() ) {
        mClient.setPassword(password);
    }

    // If you later enable MQTT auth:
    // mClient->setUsername("user");
    // mClient->setPassword("password");

    connect(&mClient, &QMqttClient::connected, this, [=]() {
        qInfo() << "MQTT connected";
        publishOnline();
        subscribe("command/down");
    });

    connect(&mClient, &QMqttClient::disconnected, this, [=]() {
        qInfo() << "MQTT disconnected";
    });

    connect(&mClient, &QMqttClient::errorChanged,
            this,
            [=](QMqttClient::ClientError error) {
                qCritical() << "MQTT error:" << error << mClient.error();
            });

    connect(&mClient, &QMqttClient::messageReceived,
            this, &Gateway::onMessageReceived);
}

void Gateway::start()
{
    qInfo() << "Mqtt client connecting to" << mMqttAddr << ":" << mMqttPort << "...";
    mClient.connectToHost();

    // Periodically send "connected" state so ChirpStack sees the gateway
    QTimer* heartbeat = new QTimer(this);
    connect(heartbeat, &QTimer::timeout, this, [this]() {
        publishOnline();

        const QString topic = QString("eu868/gateway/%1/event/stats").arg(mId);

        QJsonObject st;
        st["gatewayId"] = mId;  // MUST be gatewayId
        st["time"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

        // Minimal counters (camelCase)
        st["rxPacketsReceived"] = 0;
        st["rxPacketsReceivedOk"] = 0;
        st["txPacketsReceived"] = 0;
        st["txPacketsEmitted"] = 0;

        const QByteArray payload = QJsonDocument(st).toJson(QJsonDocument::Compact);
        mClient.publish(topic, payload, 0, false);

    });
    heartbeat->start(10000); // every 10 seconds
}

bool Gateway::publishOnline()
{
    QString topic = QString("eu868/gateway/%1/state/conn").arg(mId);

    QJsonObject root;
    root["state"] = "connected";

    QJsonDocument doc(root);
    mClient.publish(topic, doc.toJson(QJsonDocument::Compact));

    return true;
}


void Gateway::subscribe(const QString &topic)
{
    QString fullTopic = QString("eu868/gateway/%1/%2").arg(mId).arg(topic);

    QMqttSubscription* subscription = mClient.subscribe(fullTopic, 0);

    mSubscribtions.append(subscription);

    if (!subscription)
        qCritical() << "Mqtt subscription failed";
    else
        qInfo() << "Mqtt subscribtion sended..." << topic;


    // Connect to subscription stateChanged signal
    connect(subscription, &QMqttSubscription::stateChanged, this, [subscription](QMqttSubscription::SubscriptionState state){
        switch(state) {
        case QMqttSubscription::Unsubscribed:
            qInfo() << "Subscription is unsubscribed";
            break;
        case QMqttSubscription::Subscribed:
            qInfo() << "Subscription is active";
            break;
        case QMqttSubscription::Error:
            qWarning() << "Subscription error:" << subscription->reason();
            break;
        default:
            qInfo() << "Subscription pending...";
            break;
        }
    });
/*
    // Connect to messages for this subscription
    connect(subscription, &QMqttSubscription::messageReceived, this, [=](const QMqttMessage &msg){
        qInfo() << "Gateway message on topic" << msg.topic() << "payload size";
        onMessageReceived(msg.payload(), msg.topic());
    });
*/
}


// Ensure subscription is always active
void Gateway::onUpdate()
{
    process(); // existing logic
}
void Gateway::onSend()
{

}


bool Gateway::publish(const QByteArray &phyPayload)
{
    QString topic = QString("eu868/gateway/%1/event/up").arg(mId);

    QJsonObject root;

    // Base64 encode
    QString base64 = phyPayload.toBase64();

    root["phyPayload"] = base64;


    QJsonObject rx;
    rx["gatewayId"] = mId;
    rx["rssi"] = -45;
    rx["snr"] = 5.5;

    // rxInfo.context must be bytes (base64 in JSON)
    // any stable-ish blob is fine for simulation
    QString ctx = QString("%1").arg(QRandomGenerator::global()->generate());
    rx["context"] = QString(ctx.toLatin1().toBase64());
    quint32 uplinkId32 = QRandomGenerator::global()->bounded(1u, 0xFFFFFFFFu);
    rx["uplinkId"] = QJsonValue(static_cast<qint64>(uplinkId32));
    rx["crcStatus"] = "CRC_OK";

    root["rxInfo"] = rx;

    QJsonObject tx;
    tx["frequency"] = 868100000;
    tx["dr"] = 5;

    QJsonObject txLora;
    txLora["bandwidth"]= 125000;
    txLora["spreadingFactor"] = 7;
    txLora["codeRate"] = "CR_4_5";
    txLora["polarizationInversion"]=true;

    QJsonObject txModulation;
    txModulation["lora"] = txLora;
    tx["modulation"] = txModulation;

    root["txInfo"] = tx;

    QJsonDocument doc(root);

    QByteArray jsonBA = doc.toJson(QJsonDocument::Compact);
    mClient.publish( topic, jsonBA );

    return true;
}

void Gateway::onStopSending()
{
    mIsSending = false;
}


static int parseDelayMs(const QJsonObject& item) {
    const QJsonObject txInfo = item.value("txInfo").toObject();
    const QJsonObject timing = txInfo.value("timing").toObject();
    const QJsonObject delayObj = timing.value("delay").toObject();
    const QString delayStr = delayObj.value("delay").toString(); // "1s"

    if (delayStr.endsWith("ms")) {
        bool ok=false; int v = delayStr.left(delayStr.size()-2).toInt(&ok);
        return ok ? v : 0;
    }
    if (delayStr.endsWith("s")) {
        bool ok=false; int v = delayStr.left(delayStr.size()-1).toInt(&ok);
        return ok ? v*1000 : 0;
    }
    return 0;
}

void Gateway::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    const QJsonDocument doc = QJsonDocument::fromJson(message);
    const QJsonObject obj = doc.object();

    if (!topic.name().endsWith("/command/down"))
        return;

    const QJsonValue downlinkIdVal = obj.value("downlinkId");
    const QJsonArray items = obj.value("items").toArray();
    if (downlinkIdVal.isUndefined() || items.isEmpty())
        return;

    const int chosenIndex = 0; // RX1
    const QJsonObject chosenItem = items.at(chosenIndex).toObject();
    const int delayMs = parseDelayMs(chosenItem);

    const QString phyB64 = chosenItem.value("phyPayload").toString();
    const QByteArray raw = QByteArray::fromBase64(phyB64.toUtf8());

    QTimer::singleShot(delayMs, this, [=]() {

        // 1) "Transmit" to device (simulation)
        emit downlinkReceived(raw);

        // 2) Publish ACK *after* transmit attempt
        QJsonObject ack;
        ack["gatewayId"] = mId;
        ack["downlinkId"] = downlinkIdVal;

        QJsonArray ackItems;
        for (int i = 0; i < items.size(); i++) {
            QJsonObject st;
            if (i == chosenIndex) st["status"] = "OK";
            ackItems.append(st);
        }
        ack["items"] = ackItems;

        const QString ackTopic = QString("eu868/gateway/%1/event/ack").arg(mId);
        mClient.publish(ackTopic,
                        QJsonDocument(ack).toJson(QJsonDocument::Compact),
                        /*qos*/ 1,
                        /*retain*/ false);
    });
}
#else


#endif


void Gateway::process()
{

}
