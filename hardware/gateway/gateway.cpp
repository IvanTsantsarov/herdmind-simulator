#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "gateway.h"
#include "network.h"
#include "mqtt.h"
#include "defines_settings.h"
#include "simtools.h"

#ifdef SIMULATION

// #include "mainwindow.h"

Gateway::Gateway(const QSettings &settings)
{
    mId = SimTools::readStringSettingsValue(settings, MQTT_SECTION, "gatewayId");

    // If you later enable MQTT auth:
    // mClient->setUsername("user");
    // mClient->setPassword("password");

    mClient = new Mqtt(settings, this);

    connect(mClient, &Mqtt::connected, this, [&]() {
        startHeartbeat();
        subscribe("command/down");
    });

    connect( mClient, &Mqtt::messageReceived, this, &Gateway::onMessageReceived);
}

Gateway::~Gateway()
{
    delete mClient;
}

void Gateway::start()
{
    qInfo() << "Gatewaty mqtt client connecting to" << mClient->addr() << ":" << mClient->port()<< "...";
    mClient->connectToHost();
}

void Gateway::startHeartbeat()
{
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
        mClient->publish(topic, payload);
    });
    heartbeat->start(10000); // every 10 seconds
}


bool Gateway::publishOnline()
{
    QString topic = QString("eu868/gateway/%1/state/conn").arg(mId);

    QJsonObject root;
    root["state"] = "connected";

    QJsonDocument doc(root);
    mClient->publish(topic, doc.toJson(QJsonDocument::Compact));

    return true;
}


void Gateway::subscribe(const QString &topic)
{
    QString fullTopic = QString("eu868/gateway/%1/%2").arg(mId).arg(topic);
    mClient->subscribe(fullTopic);
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
    if( mClient && !mClient->isConnected() ) {
        qCritical() << "Mqtt:publish: Not connected!";
        return false;
    }

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

    return mClient->publish( topic, jsonBA );
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

void Gateway::onMessageReceived(const QByteArray &message, const QString &topicName)
{
    const QJsonDocument doc = QJsonDocument::fromJson(message);
    const QJsonObject obj = doc.object();

    if (!topicName.endsWith("/command/down"))
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
        mClient->publish(ackTopic,
                        QJsonDocument(ack).toJson(QJsonDocument::Compact) );
    });
}

#else


#endif


void Gateway::process()
{

}
