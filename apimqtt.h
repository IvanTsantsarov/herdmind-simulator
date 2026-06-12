#ifndef APIMQTT_H
#define APIMQTT_H

#include <QObject>
#include <QSettings>
#include "mqtt.h"

class DevManager;

class ApiMqtt : public Mqtt
{
    Q_OBJECT
    DevManager* devman();

public:
    explicit ApiMqtt(const QSettings& settings, DevManager* DevManager );
    bool sendMessage(const QString& eui, const QByteArray &message);

private slots:

    void onMessageReceived(const QByteArray &message,
                                   const QMqttTopicName &topic);

signals:
};

#endif // APIMQTT_H
