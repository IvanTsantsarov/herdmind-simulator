#include <QJsonArray>
#include <QJsonObject>

#include "devmanager.h"
#include "apirest.h"

void DevManager::onDevices(const QJsonObject &jobj)
{
    int count = jobj["totalCount"].toInt();

    if( States::GetDevicesCount == mState ) {
        mState = States::GetDevicesList;
        mApiRest->getDevices(count);
    }else
    if( States::GetDevicesList == mState ) {
        QJsonArray array = jobj["result"].toArray();

        qInfo() << "Devices from chirpstack received! Syncing...";

        // mark devices that is in our list and present also in chirpstack
        // and send delete request to chirpstack to devices that are not in our list
        for( auto jsonElement: array ) {
            QJsonObject jobj = jsonElement.toObject();
            QString devEUI = jobj["devEui"].toString();


            if( mDevsMap.contains(devEUI) ) {
                mSkippedDevicesCount ++;
                mDevsMap[devEUI].mIsMissing = false;
                // if the device is already in the chirpstack, mark it as present
                int index = mDevsMap[devEUI].mIndex;
                QJsonObject jobjInternal = mDevices[index].toObject();
                qInfo() << devEUI << jobjInternal["name"].toString() << "presented";
                continue;
            }


            // delete the device, because it's not in our list
            qInfo() << devEUI << jobj["name"].toString() << " to be deleted...";
            mApiRest->deleteDevice(devEUI);
            mDeletingDevicesCount ++;
        }

        qInfo() << "Adding devices to chirpstack...";

        // add mising devices
        foreach(const DevsMapValue& val, mDevsMap) {
            if( !val.mIsMissing) {
                continue;
            }

            QJsonObject jobj = mDevices[val.mIndex].toObject();
            mAddingDevicesCount ++;

            mApiRest->addDevice( jobj["name"].toString(),
                                jobj["deviceProfileId"].toString(),
                                jobj["devEui"].toString(),
                                jobj["joinEui"].toString(),
                                jobj["applicationKey"].toString() );
        }

        qInfo() << "Sync devices first stage done! Adding:" << mAddingDevicesCount
                 << "Deleting:" << mDeletingDevicesCount
                 << "Skipped:" << mSkippedDevicesCount;

    }
}

void DevManager::onDeviceAdd(const QString &devEUI)
{
    qInfo() << "Device" << devEUI << "added to Chirpstack.";
    mAddedDevicesCount ++;
    if( mAddedDevicesCount == mAddingDevicesCount ) {
        qInfo() << "Adding" << mAddedDevicesCount << "devices done!";
    }
}

void DevManager::onDeviceDel(const QString &devEUI)
{
    qInfo() << "Device" << devEUI << "deleted from Chirpstack.";

    mDeletedDevicesCount++;
    if( mDeletedDevicesCount == mDeletingDevicesCount ) {
        qInfo() << "Deleting" << mDeletingDevicesCount << "devices done!";
    }

}

void DevManager::onDeviceConf(const QString &devEUI)
{
    qInfo() << "Device" << devEUI << "configured in Chirpstack.";

    mConfiguredDevicesCount++;
    if( mAddedDevicesCount == mConfiguredDevicesCount ) {
        qInfo() << "Configuring" << mConfiguredDevicesCount << "devices done!";
    }
}



DevManager::DevManager(const QSettings &settings)
{
    // Create rest api object
    mApiRest = new ApiRest(this, settings);
}

void DevManager::syncDevices(const QByteArray &jsonList)
{
    mAddingDevicesCount = 0;
    mDeletingDevicesCount = 0;
    mAddedDevicesCount = 0;
    mSkippedDevicesCount = 0;
    mDeletedDevicesCount = 0;
    mConfiguredDevicesCount = 0;

    mDevices = QJsonDocument::fromJson( jsonList ).array();

    mDevsMap.clear();

    // Create a map with devices by their DevEUI
    int index = 0;
    foreach( auto jsonElement, mDevices ) {
        QJsonObject jobj = jsonElement.toObject();
        QString key = jobj["devEui"].toString();

        DevsMapValue val;
        val.mIndex = index ++;

        mDevsMap[key] = val;
    }

    mState = States::GetDevicesCount;
    mApiRest->getDevices();
}
