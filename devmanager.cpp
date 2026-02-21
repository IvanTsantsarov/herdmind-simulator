#include <QJsonArray>
#include <QJsonObject>

#include "devmanager.h"
#include "apirest.h"

void DevManager::onDevices(const QJsonObject& jobj)
{
    int count = jobj["totalCount"].toInt();

    if( States::GetDevicesCount == mState ) {
        mState = States::GetDevicesList;
        mApiRest->getDevices(count);
    }else
    if( States::GetDevicesList == mState ) {
        QJsonArray array = jobj["result"].toArray();

        // mark devices that is in our list and present also in chirpstack
        // and send delete request to chirpstack to devices that are not in our list
        foreach( auto jsonElement, array ) {
            QJsonObject jobj = jsonElement.toObject();
            QString devEUI = jobj["devEui"].toString();

            if( mDevsMap.contains(devEUI)) {
                // if the device is already in the chirpstack, mark it as present
                mDevsMap[devEUI].mIsMissing = false;
                continue;
            }

            // delete the device, because it's not in our list
            mApiRest->deleteDevice(devEUI);
        }

        // add mising devices
        foreach(const DevsMapValue& val, mDevsMap) {
            if( !val.mIsMissing) {
                continue;
            }

            QJsonObject jobj = mDevices[val.mIndex].toObject();

            mApiRest->addDevice( jobj["name"].toString(),
                                jobj["deviceProfileId"].toString(),
                                jobj["devEui"].toString(),
                                jobj["joinEui"].toString(),
                                jobj["applicationKey"].toString() );


        }


    }
}

DevManager::DevManager(const QSettings &settings) {
    // Create rest api object
    mApiRest = new ApiRest(this, settings);



}

void DevManager::syncDevices(const QByteArray &jsonList)
{
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
