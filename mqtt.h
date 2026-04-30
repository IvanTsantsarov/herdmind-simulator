#ifndef MQTT_H
#define MQTT_H

#include <QSettings>
#include <QMqttClient>
#include <QObject>

class Mqtt : public QObject
{
    Q_OBJECT
    QMqttClient mClient;
    QList<QMqttSubscription*> mSubscribtions;

    void updateMessages();

protected:
    QString mAddr;
    quint16 mPort = 0;
    QString mAppId;

public:
    explicit Mqtt(const QSettings &settings, QObject *parent = nullptr);
    inline const QString& addr(){ return mAddr; }
    inline quint16 port(){ return mPort; }
    inline void connectToHost(){ mClient.connectToHost(); };
    bool subscribe(const QString& topic);
    bool subscribeToDeviceUp(const QString& eui);

    quint32 publish(const QString& topic, const QByteArray& content);

    struct Message {
        enum Status {
            Sending = 0,
            Sent,
            Failed
        };

        inline bool isSending(){ return Sending == mStatus; }
        inline bool isSent(){ return Sent == mStatus; }
        inline bool isFailed(){ return Failed == mStatus; }

        Status mStatus  = Sending;
        QDateTime mTime = QDateTime::currentDateTime();
    };

    QMap<quint32, Message> mMessages;
    inline bool isConnected() { return mClient.state() == QMqttClient::Connected;    }

private slots:
    virtual void onConnected();
    virtual void onMessageSent(quint32 id);
    virtual void onMessageReceived(const QByteArray &message,
                           const QMqttTopicName &topic);
    virtual void onMessageStatusChanged(qint32 id, QMqtt::MessageStatus s, const QMqttMessageStatusProperties &properties);

signals:
    void connected();
    void messageReceived(const QByteArray& message, const QString& topicName);
};

#endif // MQTT_H
