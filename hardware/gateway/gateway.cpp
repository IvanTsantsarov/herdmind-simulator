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

    connect(&mClient, &QMqttClient::connected, this, []() {
        qInfo() << "MQTT connected";
    });

    connect(&mClient, &QMqttClient::disconnected, this, []() {
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


void Gateway::subscribe(const QString &topic)
{
    auto subscription = mClient.subscribe(topic, 0);

    if (!subscription)
        qCritical() << "Mqtt subscription failed";
    else
        qInfo() << "Mqtt Subscribed to" << topic;
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
    QString base64 =
        phyPayload.toBase64();

    root["phyPayload"] = base64;

    QJsonArray rxArray;
    QJsonObject rx;

    rx["gatewayId"] = mId;
    rx["rssi"] = -45;
    rx["snr"] = 5.5;

    rxArray.append(rx);
    root["rxInfo"] = rxArray;

    QJsonObject tx;
    tx["frequency"] = 868100000;
    tx["dr"] = 5;

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
    qDebug() << "Message received!";
    qDebug() << "Topic:" << topic.name();
    qDebug() << "Payload:" << message;

    QJsonDocument doc = QJsonDocument::fromJson(message);
    QString base64 = doc["data"].toString();
    QByteArray raw = QByteArray::fromBase64(base64.toUtf8());

    qDebug() << "Payload decoded:" << message;
    qDebug() << "Payload topic:" << topic.name();

    // If JSON:
    // QJsonDocument doc = QJsonDocument::fromJson(message);
}

#else


#endif


void Gateway::process()
{

}
