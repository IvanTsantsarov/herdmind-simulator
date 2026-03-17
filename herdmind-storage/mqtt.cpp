#include <QMqttClient>
#include <QMqttSubscription>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "mqtt.h"
#include "defines.h"


Mqtt::Mqtt(Storage *st, const QSettings &settings) :
    mClient(this), mStorage(st)
{
    mMqttAddr = settings.value(MQTT_SECTION"/address").toString();
    mMqttPort = settings.value(MQTT_SECTION"/port").toUInt();
    mAppId = settings.value(CHIRPSTACK_SECTION"/appId").toString();

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
        mStorage->onConnected();
    });

    connect(&mClient, &QMqttClient::disconnected, this, [=]() {
        qInfo() << "MQTT disconnected";
    });

    connect(&mClient, &QMqttClient::errorChanged,
            this,
            [=](QMqttClient::ClientError error) {
                qFatal() << "MQTT error:" << error << mClient.error();
            });

    connect(&mClient, &QMqttClient::messageReceived,
            this, &Mqtt::onMessageReceived);
}

void Mqtt::start()
{
    qInfo() << "Mqtt client connecting to" << mMqttAddr << ":" << mMqttPort << "...";
    mClient.connectToHost();
}


void Mqtt::subscribeToDeviceUp(const QString &devEUI)
{
    QString topic = QString("application/%1/device/%2/event/up").arg(mAppId).arg(devEUI);

    QMqttSubscription* subscription = mClient.subscribe(topic, 0);

    mSubscribtions.append(subscription);

    if (!subscription)
        qFatal() << "Mqtt subscription failed";
    else
        qInfo() << "Mqtt subscribtion sended..." << topic;


    // Connect to subscription stateChanged signal
    connect(subscription, &QMqttSubscription::stateChanged, this,
            [&,subscription](QMqttSubscription::SubscriptionState state){
        switch(state) {
        case QMqttSubscription::Unsubscribed:
            qWarning() << "MQTT:Subscription is unsubscribed:" << devEUI;
            break;
        case QMqttSubscription::Subscribed:
            qDebug() << "Subscription is active" << devEUI;
            mStorage->onSubscribed(devEUI);
            break;
        case QMqttSubscription::Error:
            qWarning() << "MQTT:Subscription error:" << subscription->reason() << devEUI;
            break;
        default:
            qInfo() << "Subscription pending...";
            break;
        }
    });
/*
    // Connect to messages for this subscription
    connect(subscription, &QMqttSubscription::messageReceived, this, [=](const QMqttMessage &msg){
        qInfo() << "Mqtt message on topic" << msg.topic() << "payload size";
        onMessageReceived(msg.payload(), msg.topic());
    });
*/
}

void Mqtt::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    const QJsonDocument doc = QJsonDocument::fromJson(message);
    const QJsonObject json = doc.object();
    mStorage->onMessage(json);
}
