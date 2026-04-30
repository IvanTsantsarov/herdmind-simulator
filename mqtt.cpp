#include "mqtt.h"
#include "simtools.h"
#include "defines_settings.h"


Mqtt::Mqtt(const QSettings &settings, QObject *parent)
    : QObject{parent}, mClient{this}
{
    mAddr = SimTools::readStringSettingsValue(settings, MQTT_SECTION,"address");
    mPort = SimTools::readIntSettingsValue(settings, MQTT_SECTION,"port");
    mAppId= SimTools::readStringSettingsValue(settings, CHIRPSTACK_SECTION,"appId");

    QString userName = SimTools::readStringSettingsValue(settings, MQTT_SECTION,"username");
    QString password = SimTools::readStringSettingsValue(settings, MQTT_SECTION,"password");

    if( !userName.isEmpty() ) {
        mClient.setUsername(userName);
    }

    if( !password.isEmpty() ) {
        mClient.setPassword(password);
    }

    // From docker-compose: mosquitto exposes 1883 to host
    mClient.setHostname(mAddr);
    mClient.setPort(mPort);

    connect(&mClient, &QMqttClient::connected, this, &Mqtt::onConnected );

    connect(&mClient, &QMqttClient::disconnected, this, [=]() {
        qInfo() << "MQTT disconnected:" << mAddr << ":" << mPort;
    });

    connect(&mClient, &QMqttClient::errorChanged,
            this,
            [=](QMqttClient::ClientError error) {
                qCritical() << "MQTT error:" << error << mClient.error();
            });

    connect(&mClient, &QMqttClient::messageReceived,
            this, &Mqtt::onMessageReceived);

    connect(&mClient, &QMqttClient::messageSent,
            this, &Mqtt::onMessageSent);

    connect(&mClient, &QMqttClient::messageStatusChanged,
            this, &Mqtt::onMessageStatusChanged);
}

bool Mqtt::subscribe(const QString &topic)
{
    QMqttSubscription* subscription = mClient.subscribe(topic, 0);

    if (!subscription) {
        qCritical() << "Mqtt subscription failed to:"<< topic;
        return false;
    }

    mSubscribtions.append(subscription);

    qInfo() << "Mqtt subscribtion sended:" << topic;

    // Connect to subscription stateChanged signal
    connect(subscription, &QMqttSubscription::stateChanged, this, [&,subscription](QMqttSubscription::SubscriptionState state){
        switch(state) {
        case QMqttSubscription::Unsubscribed:
            qInfo() << "Mqtt Subscription is unsubscribed";
            break;
        case QMqttSubscription::Subscribed:
            qInfo() << "Mqtt Subscription is active";
            break;
        case QMqttSubscription::Error:
            qWarning() << "Mqtt Subscription error:" << subscription->reason();
            break;
        default:
            qInfo() << "Mqtt Subscription pending...";
            break;
        }
    });

    return true;
}

bool Mqtt::subscribeToDeviceUp(const QString &eui)
{
    QString topic = QString("application/%1/device/%2/event/up").arg(mAppId).arg(eui);
    return subscribe(topic);
}

quint32 Mqtt::publish(const QString &topic, const QByteArray &content)
{
    quint32 id = mClient.publish( topic, content, 1, false );

    Message msg;
    if( 0 == id) {
        qCritical() << "Mqtt:publish: Message with zero ID is not tracked";
        return 0;
    }

    if( mMessages.contains(id)) {
        qCritical() << "Mqtt:publish: Message with this ID already exists in the mMessages:" << id;
    }else {
        mMessages[id] = msg;
    }

    return id;
}

void Mqtt::onConnected()
{
    qInfo() << "MQTT connected:" << mAddr << ":" << mPort;
    emit connected();
#ifdef SIMULATION
    // gMainWindow->onMqttConnected();
#endif

}

void Mqtt::onMessageSent(quint32 id)
{
    if( !mMessages.contains(id) ) {
        return;
    }

    Message& msg = mMessages[id];
    qDebug() << "Mqtt message from" << msg.mTime.toString("hh:mm:ss.zzz") << "received by the broker";
    msg.mStatus = Message::Status::Sent;
}

void Mqtt::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    emit messageReceived( message, topic.name() );
}


void Mqtt::onMessageStatusChanged(qint32 id, QMqtt::MessageStatus s, const QMqttMessageStatusProperties &properties)
{
    QString statusStr;
    switch(s) {
    case QMqtt::MessageStatus::Unknown: statusStr = "Unknown"; break;
    case QMqtt::MessageStatus::Published: statusStr = "Published"; break;
    case QMqtt::MessageStatus::Acknowledged: statusStr = "Acknowledged"; break;
    case QMqtt::MessageStatus::Received: statusStr = "Received"; break;
    case QMqtt::MessageStatus::Released: statusStr = "Released"; break;
    case QMqtt::MessageStatus::Completed: statusStr = "Completed"; break;
    }

    qDebug() << "Mqtt Message status changed ID:" << id << "to:" << statusStr << "(" << (int)s << ")";

    updateMessages();
}

void Mqtt::updateMessages()
{
    int sending = 0;
    int sent = 0;
    int failed = 0;

    QList<quint32> ids = mMessages.keys();
    for( quint32 id: ids) {
        Message& m = mMessages[id];
        switch (m.mStatus) {
        case Message::Status::Sending :
            sending ++;
            break;
        case Message::Status::Sent :
            sent ++;
            mMessages.remove(id);
            break;
        case Message::Status::Failed :
            failed ++;
            break;
        default:
            break;
        }
    }

    qDebug() << "Mqtt client sending:" << sending << "sent:" << sent << "failed:" << failed;
}
