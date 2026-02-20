#include "apirest.h"
#include "mainwindow.h"
#include "qjsonparseerror.h"
#include "devmanager.h"

void ApiRest::onResponse()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "REST:Request failed:" << reply->errorString() << responseData;
        qCritical() << "REST:Network failure!";
        return;
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError && jsonDoc.isObject()) {
        qDebug() << "REST:Failed to parse JSON:" << parseError.errorString();
        qCritical() << "REST:JSON parse error!";
        return;
    }

    auto json = jsonDoc.object();
    qDebug() << QJsonDocument(json).toJson(QJsonDocument::Indented); // Uncomment for Debug

    RequestType type = static_cast<RequestType>( reply->property("requestType").toInt() );

    switch(type) {
        case RequestType::None: break;
        case RequestType::GetDevices: onGetDevicesResponse(json); break;
        case RequestType::DeleteDevice: onGetDevicesResponse(json); break;
        case RequestType::AddDevice: onAddDeviceResponse(json); break;
        }
}

void ApiRest::onGetDevicesResponse(QJsonObject& jobj)
{
    mDevManager->onDevices(0);
}

void ApiRest::onDeleteDeviceResponse(QJsonObject &jobj)
{

}

void ApiRest::onAddDeviceResponse(QJsonObject &jobj)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    QByteArray devEUI = reply->property("devEUI").toByteArray();
    QByteArray joinEUI = reply->property("joinEUI").toByteArray();
    QByteArray nwkKey = reply->property("nwkKey").toByteArray();

    setDeviceKeys(devEUI, joinEUI, nwkKey);
}

void ApiRest::onSetDeviceKeysResponse(QJsonObject &jobj)
{

}


