#include <QJsonArray>
#include <QJsonObject>


#include "hardware/gateway/gateway.h"
#include "network.h"
#include "mainwindow.h"
#include "devmanager.h"
#include "apirest.h"
#include "herd.h"


///////////////////////////////////////////////////////////////////////////
// Currently supporting only ABP (Activation By Personalization)
// OTAA (Over the Air Activation) NOT SUPPORTED!
///////////////////////////////////////////////////////////////////////////


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
        for( const auto& jsonElement: array ) {
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

        // if all devices are skipped call onDevicesReady
        if( mSkippedDevicesCount == count ) {
            onDevicesReady();
            return;
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
                                jobj["address"].toString() );
        }

        qInfo() << "Sync devices first stage finished!";
        qInfo() << "Adding:" << mAddingDevicesCount;
        qInfo() << "Deleting:" << mDeletingDevicesCount;
        qInfo() << "Skipped:" << mSkippedDevicesCount;

    }
}

void DevManager::onDeviceAdd(const QString &devEUI)
{
    qInfo() << "Device" << devEUI << "added to Chirpstack.";
    mAddedDevicesCount ++;
    if( mAddedDevicesCount == mAddingDevicesCount ) {
        qInfo() << "Adding (ABP)" << mAddedDevicesCount << "devices done!";
    }

    mApiRest->getDeviceAddress(devEUI);
}


// Obtain and store device addres and call activate device
void DevManager::onDeviceAddress(const QString &devEUI, const QString &devAddr)
{
    if( !mDevsMap.contains(devEUI) ) {
        qWarning() << "Device" << devEUI << " is missing, but address obtained:" << devAddr;
        return;
    }

    LoraDev* dev = gMainWindow->herd()->device(devEUI);

    if( nullptr == dev) {
        qWarning() << "Device" << devEUI << " missing in the herd";
        return;
    }

    bool isOk = false;
    uint32_t a = devAddr.toUInt(&isOk, 16);

    if( !isOk) {
        qWarning() << "Device" << devEUI << " address invalid number:" << devAddr;
        return;
    }

    dev->setAddress(a);

    qInfo() << "Device" << devEUI << " address obtained:" << devAddr;

    mApiRest->activateDevice(devEUI, devAddr, dev->appKey().toHex());
}

void DevManager::onDeviceDel(const QString &devEUI)
{
    qInfo() << "Device" << devEUI << "deleted from Chirpstack.";

    mDeletedDevicesCount++;
    if( mDeletedDevicesCount == mDeletingDevicesCount ) {
        qInfo() << "Deleting" << mDeletingDevicesCount << "devices done!";
    }

}

void DevManager::onDeviceActivated(const QString &devEUI)
{
    qInfo() << "Device" << devEUI << "activated.";

    mActivatedDevicesCount++;
    if( mAddedDevicesCount == mActivatedDevicesCount ) {
        qInfo() << "Activated" << mActivatedDevicesCount << "devices done!";
        onDevicesReady();
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
    mActivatedDevicesCount = 0;

    mDevices = QJsonDocument::fromJson( jsonList ).array();

    mDevsMap.clear();

    // Create a map with devices by their DevEUI
    int index = 0;
    for( const auto& jsonElement: mDevices ) {
        QJsonObject jobj = jsonElement.toObject();
        QString key = jobj["devEui"].toString();

        DevsMapValue val;
        val.mIndex = index ++;

        mDevsMap[key] = val;
    }

    mState = States::GetDevicesCount;
    mApiRest->getDevices();
}


void DevManager::onDevicesReady()
{
    gMainWindow->network()->edge()->start();
}
