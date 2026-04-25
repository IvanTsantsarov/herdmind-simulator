#ifndef DEVMANAGER_H
#define DEVMANAGER_H

#include <QGeoCoordinate>
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
        GetGatewaysCount,
        GetGatewaysList,
        SetupFence
    };

    States mState = States::None;

    ApiRest* mApiRest = nullptr;

    QList<LoraDev*> mDevicesList;
    QMap<QString, LoraDev*> mDevicesMap;

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
    int mSetupDevicesCount = 0;

    int mCollarsCount = 0;
    int mBolusesCount = 0;

    Gateway *mEdge = nullptr;

protected:
    void onDevices(const QJsonObject &jobj);
    void onDeviceAdd(const QString& devEUI);
    void onDeviceDel(const QString& devEUI);
    void onDeviceActivated(const QString& devEUI);
    void onDeviceAddress(const QString& devEUI, const QString& devAddr);

    void onDevicesReady(bool isStore);
    void onGateways(const QJsonObject &jobj);

public:
    DevManager( const QSettings& settings );
    inline bool isReady(){ return mIsDevicesReady; }
    inline int collarsCount(){ return mCollarsCount; }
    inline int bolusesCount(){ return mBolusesCount; }
    void syncDevices(const QByteArray &jsonList, QList<LoraDev*> devs, Gateway *edge);
    LoraDev* device(const QString& eui);
    QString deviceName(const QString& eui);
    QList<LoraDev*> devices(){ return mDevicesList; }
    inline int devicesCount(){ return mDevicesList.count(); }
    bool sendMessage(const QString& eui, const QByteArray& msg);
    bool setupFence(const QVector<QGeoCoordinate>& coords);
};

#endif // DEVMANAGER_H
