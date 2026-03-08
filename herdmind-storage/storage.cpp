#include "storage.h"
#include "apirest.h"
#include <QGeoCoordinate>
#include "defines.h"
#include "mqtt.h"
#include "tools.h"
#include "../hardware/protocol.h"

Storage::Storage(QSettings& settings, QObject *parent)
    : QObject{parent}
{
    mApiRest = new ApiRest(this, settings);
    mMqtt = new Mqtt(this, settings);

    mBolusProfile = settings.value(CHIRPSTACK_SECTION"/bolusProfileId").toString();
    mCollarProfile = settings.value(CHIRPSTACK_SECTION"/collarProfileId").toString();
}

void Storage::run()
{
    mApiRest->getDevices(MAX_DEVICES_COUNT);
    mMqtt->start();
}

void Storage::subscribe()
{
    for( const QString& eui: mDevices.keys()) {
        mMqtt->subscribeToDeviceUp(eui);
    }
}

void Storage::registerDevices(QJsonObject &jobj)
{
    int count = jobj["totalCount"].toInt();

    if( !mHasDevicesCouns ) {
        mHasDevicesCouns = true;
        mApiRest->getDevices(count);
    }else
    {
        QJsonArray array = jobj["result"].toArray();

        qInfo() << "Devices from chirpstack received! Syncing...";

        // mark devices that is in our list and present also in chirpstack
        // and send delete request to chirpstack to devices that are not in our list
        for( const auto& jsonElement: array ) {
            QJsonObject jobj = jsonElement.toObject();
            QString devEui = jobj["devEui"].toString();

            DeviceProfile p = DeviceProfile::None;
            QString profileStr = jobj["deviceProfileId"].toString();
            if( profileStr == mBolusProfile) {
                p = DeviceProfile::Bolus;
            }
            else
            if( profileStr == mCollarProfile) {
                p = DeviceProfile::Collar;
            }

            Device dev( devEui,
                       jobj["address"].toString(),
                       jobj["name"].toString(),
                       p );

            mDevices.insert(devEui, dev);
            qInfo() << "Added device:" << dev.name() << dev.eui();
        }

        mHasDevices = true;

        if( mIsConnected && !mIsSubscribing) {
            subscribe();
        }

        qInfo() << "All devices obtained!";
    }
}

void Storage::onConnected()
{
    mIsConnected = true;

    if( mHasDevices && !mIsSubscribing) {
        subscribe();
    }
}

void Storage::onSubscribed(const QString &eui)
{
    if( !mDevices.contains(eui) ) {
        qWarning() << "Device subscribed but not registered:" << eui;
        return;
    }

    mSubscribedCount ++;
    mDevices[eui].subscribe();
    qInfo() << "Device subscribed:" << eui;

    if( mSubscribedCount == mDevices.count() ) {
        qInfo() << "All" << mSubscribedCount << "devices subscribed!";
        return;
    }

    if( mSubscribedCount >= mDevices.count() )  {
        qWarning() << "More device than registerd is subscribed:" << eui;
    }
}

void Storage::onMessage(const QJsonObject &jobj)
{
    QJsonObject deviceInfo = jobj.value("deviceInfo").toObject();
    QString eui = deviceInfo.value("devEui").toString();

    if( !mDevices.contains(eui) ) {
        qDebug() << "Received message from unregistered device:" << eui;
        return;
    }

    Device dev = mDevices[eui];

    QByteArray payload = QByteArray::fromBase64( jobj.value("data").toString().toUtf8() );

    // parse here the data
    switch (dev.profile()) {
    case DeviceProfile::Bolus: {
        Protocol::Bolus package;
        package.fromByteArray((uint8_t*)payload.constData());
        QString cond;
        if( package.isOk() ) {
            cond = "normal.";
        }else {
            if( package.hasAtony() ) {
                cond = "Atony";
            }

            if( package.hasHiper() ) {
                if( cond.length() ) cond += ",";
                cond += "Hiper";
            }

            if( package.hasHypo() ) {
                if( cond.length() ) cond += ",";
                cond += "Hypo";
            }

            if( package.hasTemp() ) {
                if( cond.length() ) cond += ",";
                cond += "HTemp";
            }
        }
        qInfo() << Tools::deviceTimestampString(package.mTimestamp) << dev.name() << cond;
    }
    break;
    case DeviceProfile::Collar: {
        Protocol::Collar package;
        package.fromByteArray((uint8_t*)payload.constData());
        QGeoCoordinate coord(package.decodeLat(), package.decodeLon());
        qInfo() << Tools::deviceTimestampString(package.mTimestamp) << dev.name() << coord.toString(QGeoCoordinate::Degrees);
    }
    break;
    default:
        break;
    }

    // qDebug() << "A message from" << eui << payload;
}
