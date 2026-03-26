#pragma once

#include <QMqttClient>
#include <QSettings>
#include <QTimer>
#include <QObject>

#include "storage.h"

class QMqttClient;
class QMqttSubscription;

class Mqtt : public QObject {

    Q_OBJECT

    QString mMqttAddr;
    quint16 mMqttPort = 0;
    QString mAppId;

    QTimer mPullTimer;

    QMqttClient mClient;
    QList<QMqttSubscription*> mSubscribtions;

    Storage* mStorage = nullptr;

    void sendPullData();

public:

    Mqtt(Storage* st, const QSettings &settings);


    void start();
    void subscribeToDeviceUp(const QString &devEUI);

signals:
    void downlinkReceived(const QByteArray& response);

protected slots:
    void onUplinkReceived(const QByteArray &message,
                           const QMqttTopicName &topic);
};


