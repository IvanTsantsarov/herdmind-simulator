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

            if( mDevsMapJson.contains(devEUI) ) {
                mSkippedDevicesCount ++;
                mDevsMapJson[devEUI].mIsMissing = false;
                // if the device is already in the chirpstack, mark it as present
                int index = mDevsMapJson[devEUI].mIndex;
                QJsonObject jobjInternal = mDevicesJson[index].toObject();
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
            onDevicesReady(false);
            return;
        }

        qInfo() << "Adding devices to chirpstack...";

        // add mising devices
        foreach(const DevsMapValue& val, mDevsMapJson) {
            if( !val.mIsMissing) {
                continue;
            }

            QJsonObject jobj = mDevicesJson[val.mIndex].toObject();
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
    qInfo() << "Device" << devEUI << device(devEUI)->name() << "added to Chirpstack.";
    mAddedDevicesCount ++;
    if( mAddedDevicesCount == mAddingDevicesCount ) {
        qInfo() << "Adding (ABP)" << mAddedDevicesCount << "devices done!";
    }

    mApiRest->getDeviceAddress(devEUI);
}


// Obtain and store device addres and call activate device
void DevManager::onDeviceAddress(const QString &devEUI, const QString &devAddr)
{
    if( !mDevsMapJson.contains(devEUI) ) {
        qWarning() << "Device" << devEUI << " is missing, but address obtained:" << devAddr;
        return;
    }

    LoraDev* dev = device(devEUI);

    if( nullptr == dev) {
        qWarning() << "Device" << devEUI << " missing in the herd";
        return;
    }

    bool isOk = false;
    uint32_t a = devAddr.toUInt(&isOk, 16);

    if( !isOk) {
        qWarning() << "Device" << devEUI << device(devEUI)->name() << " address invalid number:" << devAddr << a;
        return;
    }

    dev->setAddress(QByteArray::fromHex( devAddr.toLatin1() ));

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
    LoraDev* dev = device(devEUI);
    qInfo() << "Device" << devEUI << dev->name() << "activated with address:" << dev->addr().toHex();

    mActivatedDevicesCount++;
    if( mAddedDevicesCount == mActivatedDevicesCount ) {
        qInfo() << "Activated" << mActivatedDevicesCount << "devices done!";
        onDevicesReady(true);
    }
}



DevManager::DevManager(const QSettings &settings)
{
    // Create rest api object
    mApiRest = new ApiRest(this, settings);
}

void DevManager::syncDevices( const QByteArray &jsonList, QList<LoraDev *> devs, Gateway* edge )
{
    mAddingDevicesCount = 0;
    mDeletingDevicesCount = 0;
    mAddedDevicesCount = 0;
    mSkippedDevicesCount = 0;
    mDeletedDevicesCount = 0;
    mActivatedDevicesCount = 0;

    mDevicesJson = QJsonDocument::fromJson( jsonList ).array();

    mDevsMapJson.clear();

    mDevices.clear();
    for( LoraDev* dev: devs) {
        mDevices[dev->eui().toHex()] = dev;
        dev->setGateway(edge);
    }

    // Create a map with devices by their DevEUI
    int index = 0;
    for( const auto& jsonElement: mDevicesJson ) {
        QJsonObject jobj = jsonElement.toObject();
        QString key = jobj["devEui"].toString();

        DevsMapValue val;
        val.mIndex = index ++;

        mDevsMapJson[key] = val;
    }

    mState = States::GetDevicesCount;
    mApiRest->getDevices();
}


LoraDev *DevManager::device(const QString &devEUI)
{
    if( !mDevices.contains(devEUI) ) {
        return nullptr;
    }

    return mDevices[devEUI];
}

QString DevManager::deviceName(const QString &eui)
{
    LoraDev* dev = device(eui);
    if( dev ) {
        return dev->name();
    }

    return QString();
}

void DevManager::sendMessage(const QString &eui, const QByteArray &msg)
{
    LoraDev* dev = device(eui);

    if( dev ) {
        qInfo() << "Sending to device:" << device(eui)->name() << msg;
    }else {
        qCritical() << "Sending to unknown device:" << msg;
    }

    mApiRest->sendDeviceMessage(eui, msg);
}


void DevManager::onDevicesReady(bool isStore)
{
    gMainWindow->onDevicesReady(isStore);
    gMainWindow->network()->edge()->start();
}
