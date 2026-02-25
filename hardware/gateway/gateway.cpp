#include <QMqttClient>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "gateway.h"
#include "defines_settings.h"

#ifdef SIMULATION

Gateway::Gateway(const QSettings &settings)
{
    mMqttAddr = settings.value(MQTT_SECTION"/address").toString();
    mMqttPort = settings.value(MQTT_SECTION"/port").toUInt();
    mId = settings.value(MQTT_SECTION"/gatewayId").toString();

    mClient = new QMqttClient(this);

    // From docker-compose: mosquitto exposes 1883 to host
    mClient->setHostname(mMqttAddr);
    mClient->setPort(mMqttPort);

    // If you later enable MQTT auth:
    // mClient->setUsername("user");
    // mClient->setPassword("password");

    connect(mClient, &QMqttClient::connected, this, []() {
        qDebug() << "MQTT connected";
    });

    connect(mClient, &QMqttClient::disconnected, this, []() {
        qDebug() << "MQTT disconnected";
    });

    connect(mClient, &QMqttClient::errorChanged,
            this,
            [](QMqttClient::ClientError error) {
                qDebug() << "MQTT error:" << error;
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

    mClient->publish( topic, doc.toJson(QJsonDocument::Compact) );

    return true;
}
void Gateway::onStopSending()
{
    mIsSending = false;
}

#else


#endif


void Gateway::process()
{

}
