#ifndef STORAGE_H
#define STORAGE_H

#include <QSettings>
#include <QObject>
#include <QMap>

class ApiRest;
class Mqtt;

class Storage : public QObject
{
    Q_OBJECT

    friend class ApiRest;
    friend class Mqtt;

    ApiRest* mApiRest = nullptr;
    Mqtt* mMqtt = nullptr;

    bool mHasDevicesCouns = false;
    bool mIsConnected = false;
    bool mIsSubscribing = false;
    bool mIsSubscribed = false;
    bool mHasDevices = false;
    int mSubscribedCount = 0;
    QString mBolusProfile, mCollarProfile;

    enum struct DeviceProfile {
        None = 0,
        Bolus = 1,
        Collar = 2
    };

    class Device {

        bool mIsSubscribed = false;
        QString mEui;
        QString mAddress;
        QString mName;
        DeviceProfile mProfile = DeviceProfile::None;

    public:
        Device(){}
        Device(const QString& eui, const QString& a, const QString& n, DeviceProfile p)
            : mEui(eui), mAddress(a), mName(n), mProfile(p) {}

        inline QString name() const { return mName; }
        inline QString addr() const{ return mAddress; }
        inline QString eui() const{ return mEui; }
        inline DeviceProfile profile() const{ return mProfile; }
        inline bool isValid() const{ return mProfile != DeviceProfile::None; }
        inline bool isCollar() const{ return mProfile == DeviceProfile::Collar; }
        inline bool isBolus(){ return mProfile == DeviceProfile::Bolus; }
        inline bool isSubscribed() const{ return mIsSubscribed; }

        void subscribe(){ mIsSubscribed = true; }

    };

    QMap<QString, Device> mDevices; // the key is EUI

    void subscribe();

protected:
    void registerDevices(QJsonObject& jobj);
    void onMessage(const QJsonObject &jobj);
    void onConnected();
    void onSubscribed(const QString& eui);

public:
    explicit Storage(QSettings &settings, QObject *parent = nullptr);

    void run();



signals:
};

#endif // STORAGE_H
