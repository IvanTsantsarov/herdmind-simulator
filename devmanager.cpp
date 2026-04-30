#include <QJsonArray>
#include <QJsonObject>

#include "hardware/gateway/gateway.h"
#include "hardware/protocol.h"
#include "network.h"
#include "mainwindow.h"
#include "devmanager.h"
#include "apirest.h"
#include "apimqtt.h"
#include "herd.h"

///////////////////////////////////////////////////////////////////////////
// Currently supporting only ABP (Activation By Personalization)
// OTAA (Over the Air Activation) NOT SUPPORTED!
// All devices are setup using Rest API (mApiRest)
// Further communitions is made trough MQTT (mApiMqtt)
///////////////////////////////////////////////////////////////////////////


DevManager::DevManager(const QSettings &settings)
{
    // Create rest api object
    mApiRest = new ApiRest(settings, this);

    // Create Mqtt client class
    mApiMqtt = new ApiMqtt(settings, this);

    connect( mApiMqtt, &Mqtt::connected, this, &DevManager::onConnectedMqtt );
}

DevManager::~DevManager()
{
    delete mApiRest;
    delete mApiMqtt;
}


void DevManager::subscribeToDevicesUp()
{
    if( mIsSubscribedToDevicesUp ) {
        return;
    }

    // subscribe to all devices messages to the chirpstack
    for( const QString& eui: mDevicesMap.keys()) {
        mApiMqtt->subscribeToDeviceUp(eui);
    }

    mIsSubscribedToDevicesUp = true;
}

void DevManager::onDevices(const QJsonObject &jobj)
{
    int count = jobj["totalCount"].toInt();

    if( States::GetDevicesCount == mState ) {
        mState = States::GetDevicesList;
        mApiRest->getDevices(count);
        return;
    }

    Q_ASSERT( States::GetDevicesList == mState );

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
    if( (count && (mSkippedDevicesCount == count)) || mDevsMapJson.empty() ) {
        onDevicesReady(false);
    } else {

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
                                jobj["devEui"].toString() );
        }

        qInfo() << "Sync devices first stage finished!";
        qInfo() << "Adding:" << mAddingDevicesCount;
        qInfo() << "Deleting:" << mDeletingDevicesCount;
        qInfo() << "Skipped:" << mSkippedDevicesCount;
    }

    mState = States::GetGatewaysCount; // trash comment
    mApiRest->getGateways(); // trash comment
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

    mApiRest->activateDevice(devEUI, devAddr, dev->appSKey().toHex(), dev->nwkSKey().toHex());
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




void DevManager::syncDevices( const QByteArray &jsonList, QList<LoraDev *> devs, Gateway* edge )
{
    mEdge = edge;
    mAddingDevicesCount = 0;
    mDeletingDevicesCount = 0;
    mAddedDevicesCount = 0;
    mSkippedDevicesCount = 0;
    mDeletedDevicesCount = 0;
    mActivatedDevicesCount = 0;
    mCollarsCount = 0;
    mBolusesCount = 0;

    mDevicesList = devs;
    mDevicesJson = QJsonDocument::fromJson( jsonList ).array();

    mDevsMapJson.clear();

    mDevicesMap.clear();
    for( LoraDev* dev: devs) {
        mDevicesMap[dev->eui().toHex()] = dev;
        dev->setGateway(mEdge);
        if( dev->isBolus() ) {
            mBolusesCount++;
        }else
        if( dev->isCollar()) {
            mCollarsCount++;
        }
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
    if( !mDevicesMap.contains(devEUI) ) {
        return nullptr;
    }

    return mDevicesMap[devEUI];
}

QString DevManager::deviceName(const QString &eui)
{
    LoraDev* dev = device(eui);
    if( dev ) {
        return dev->name();
    }

    return QString();
}

bool DevManager::sendMessageRest(const QString &eui, const QByteArray &msg)
{
    LoraDev* dev = device(eui);

    if( !dev ) {
        qCritical() << "Sending to unknown device:" << msg;
        return false;
    }

    qInfo() << "Sending to device:" << dev->name() << msg;
    mApiRest->sendDeviceMessage(eui, msg, dev->fport());
    return true;
}

bool DevManager::sendMessageMqtt(const QString &eui, const QByteArray &msg)
{
    return mApiMqtt->sendMessage(eui, msg);
}

void DevManager::onDevicesReady(bool isStore)
{
    Q_ASSERT(mEdge);

    mIsDevicesReady = true;
    gMainWindow->onDevicesReady(isStore);
    mEdge->start();

    if( mApiMqtt->isConnected() ) {
        subscribeToDevicesUp();
    }
}

void DevManager::onGateways(const QJsonObject &jobj)
{
    Q_ASSERT(mEdge);

     int count = jobj["totalCount"].toInt();

    if( States::GetGatewaysCount == mState ) {
         if( count < 1 ) {
            qCritical() << "*** No Gateway registered for tenant" << mApiRest->tenantId();
            return;
         }

        mState = States::GetGatewaysList;
        mApiRest->getGateways(count);
        return;
    }

    Q_ASSERT(States::GetGatewaysList == mState);

    QJsonArray array = jobj["result"].toArray();

    qInfo() << "Gateways from chirpstack count:" << count;

    bool hasGateway = false;
    for( const auto& jsonElement: array ) {
        QJsonObject jobj = jsonElement.toObject();
        QString gatewayId = jobj["gatewayId"].toString();
        qInfo() << jobj["name"].toString() << gatewayId;
        if( mEdge->id() == gatewayId ) {
            hasGateway = true;
        }
    }

    if( !hasGateway) {
        qCritical() << "*** Gateway" << mEdge->id()
                    <<" not registered for tenant" << mApiRest->tenantId()
                    << "Please add it to Chirpstack UI!";
    }else {
        qInfo() << "Proper Gateway found!" << mEdge->id();
    }
}


// Sent with sendMessageMqtt
void DevManager::onDeviceMessageMqtt(const QString &devEUI, const QByteArray &msg)
{

}


bool DevManager::setupFence(const QVector<QGeoCoordinate> &coords)
{
    QVector<uint32_t> newCoords;
    newCoords.reserve(coords.count() + 1);
    newCoords.append(coords.count());

    for( QGeoCoordinate c: coords) {
        newCoords.append( Protocol::encodeLat(c.latitude()) );
        newCoords.append( Protocol::encodeLat(c.longitude()) );
    }

    uint32_t dataSize = sizeof(uint32_t) * newCoords.count();
    QByteArray ba(dataSize, 0);
    uint8_t* data = reinterpret_cast<uint8_t*>(ba.data());
    data[0] = static_cast<uint8_t>(Protocol::Collar::Event::SetupFence);

    for( uint32_t i = 0; i < newCoords.count(); i ++) {
        Protocol::writeUint32( newCoords[i], data, i*sizeof(uint32_t) );
    }

    for( LoraDev* dev: mDevicesList) {
        if( dev->isCollar() ) {
            // sendMessageRest(dev->eui(), ba);
            sendMessageMqtt(dev->eui().toHex(), ba);
        }
    }

    mState = States::SetupFence;
    mSetupDevicesCount = 0;

    return true;
}

void DevManager::onConnectedMqtt()
{
    if( mIsDevicesReady ) {
        subscribeToDevicesUp();
    }
}
