#ifndef DEVMANAGER_H
#define DEVMANAGER_H

#include <QSettings>
#include <QJsonArray>

class ApiRest;

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


    // array of saved devices in the device.json
    // from previous generation here in the simulator
    QJsonArray mDevices;

    struct DevsMapValue {
        int mIndex;
        bool mIsMissing = true;
    };

    // int is an index in mDevices json array
    QMap<QString, DevsMapValue> mDevsMap;


protected:
    void onDevices(const QByteArray &response);
public:
    DevManager( const QSettings& settings );

    void syncDevices(const QByteArray &jsonList);
};

#endif // DEVMANAGER_H
