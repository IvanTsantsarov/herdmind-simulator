#ifndef DEVMANAGER_H
#define DEVMANAGER_H

#include <QSettings>
#include <QJsonArray>


class ApiRest;
class LoraDev;
class Gateway;

class DevManager
{
    friend class ApiRest;

    enum struct States {
        None = 0,
        GetDevicesCount,
        GetDevicesList,
    };

    States mState = States::None;

    ApiRest* mApiRest = nullptr;

    QMap<QString, LoraDev*> mDevices;

    // array of saved devices in the device.json
    // from previous generation here in the simulator
    QJsonArray mDevicesJson;

    struct DevsMapValue {
        int mIndex;
        bool mIsMissing = true;
    };

    // int is an index in mDevices json array
    QMap<QString, DevsMapValue> mDevsMapJson;

    int mAddingDevicesCount = 0;
    int mDeletingDevicesCount = 0;
    int mAddedDevicesCount = 0;
    int mSkippedDevicesCount = 0;
    int mDeletedDevicesCount = 0;
    int mActivatedDevicesCount = 0;
    bool mIsDevicesReady = false;

protected:
    void onDevices(const QJsonObject &jobj);
    void onDeviceAdd(const QString& devEUI);
    void onDeviceDel(const QString& devEUI);
    void onDeviceActivated(const QString& devEUI);
    void onDeviceAddress(const QString& devEUI, const QString& devAddr);

    void onDevicesReady(bool isStore);

public:
    DevManager( const QSettings& settings );
    inline bool isReady(){ return mIsDevicesReady; }
    void syncDevices(const QByteArray &jsonList, QList<LoraDev*> devs, Gateway *edge);
    LoraDev* device(const QString& eui);
    QString deviceName(const QString& eui);
    QList<QString> devices(){ return mDevices.keys(); }
    void sendMessage(const QString& eui, const QByteArray& msg);
};

#endif // DEVMANAGER_H
