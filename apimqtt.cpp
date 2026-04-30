#include <QJsonObject>
#include "apimqtt.h"
#include "devmanager.h"
#include "mqtt.h"
#include "hardware/loradev.h"

DevManager *ApiMqtt::devman()
{
    return static_cast<DevManager*>(parent());
}

ApiMqtt::ApiMqtt(const QSettings &settings, DevManager *DevManager)
    : Mqtt{settings, DevManager}
{
    connectToHost();
}

bool ApiMqtt::sendMessage(const QString &eui, const QByteArray &message)
{
    QJsonObject obj;
    obj["confirmed"] = false;
    obj["fPort"] = devman()->device(eui)->fport();
    obj["data"] = QString::fromLatin1(message.toBase64());

    QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QString topic = QString("application/%1/device/%2/command/down")
                        .arg(mAppId)
                        .arg(eui);

    qDebug() << "MQTT downlink topic:" << topic << json;

    return publish(topic, json);}

void ApiMqtt::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    // devman()->onDeviceMessageMqtt()
}