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

    // From docker-compose: mosquitto exposes 1883 to host
    mClient.setHostname(mMqttAddr);
    mClient.setPort(mMqttPort);

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
}

bool Gateway::publishOnline()
{
    QString topic = QString("eu868/gateway/%1/state/conn").arg(mId);

    QJsonObject root;
    root["state"] = "online";

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

    // Connect to messages for this subscription
    connect(subscription, &QMqttSubscription::messageReceived, this, [=](const QMqttMessage &msg){
        qDebug() << "Gateway message on topic" << msg.topic() << "payload size" << msg.payload().size() << msg.payload();
    });
}


void Gateway::onUpdate()
{
    process();
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

    root["rxInfo"] = rx;

    QJsonObject tx;
    tx["frequency"] = 868100000;
    tx["dr"] = 5;

    QJsonObject txLora;
    txLora["bandwidth"]= 125000;
    txLora["spreadingFactor"] = 7;
    txLora["codeRate"] = "CR_4_5";

    QJsonObject txModulation;
    txModulation["lora"] = txLora;
    tx["modulation"] = txModulation;

    root["txInfo"] = tx;

    QJsonDocument doc(root);

    mClient.publish( topic, doc.toJson(QJsonDocument::Compact) );

    return true;
}

void Gateway::onStopSending()
{
    mIsSending = false;
}


void Gateway::onMessageReceived(const QByteArray &message,
                               const QMqttTopicName &topic)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject obj = doc.object();

    qDebug() << "Gateway received:" << topic.name() << doc.toJson();

    QJsonArray payloads = obj["items"].toArray();

    if( payloads.isEmpty() ) {
        qInfo() << "Payload is empty";
    }else {
        for( auto payload: payloads) {
            QJsonObject jobj = payload.toObject();
            QString base64 = jobj["phyPayload"].toString();
            QByteArray raw = QByteArray::fromBase64(base64.toUtf8());
            qDebug() << "Payload:" << raw;

            // send it to all devices
            emit downlinkReceived(raw);
        }
    }
}

#else


#endif


void Gateway::process()
{

}
