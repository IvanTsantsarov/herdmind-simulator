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
        qCritical() << "REST:Network failure!" << reply->errorString() << responseData;
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
        case RequestType::DeleteDevice: onDeleteDeviceResponse(json); break;
        case RequestType::AddDevice: onAddDeviceResponse(json); break;
        case RequestType::GetDeviceAddress: onGetDeviceAddress(json); break;
        case RequestType::ActivateDevice: onActivateResponse(json); break;
        }
}

void ApiRest::onGetDevicesResponse(QJsonObject& jobj)
{
    mDevManager->onDevices(jobj);
}

void ApiRest::onDeleteDeviceResponse(QJsonObject &jobj)
{
    Q_UNUSED(jobj);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString devEUI = reply->property("devEUI").toString();
    mDevManager->onDeviceDel(devEUI);
}

void ApiRest::onAddDeviceResponse(QJsonObject &jobj)
{
    Q_UNUSED(jobj);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString devEUI = reply->property("devEUI").toString();
    mDevManager->onDeviceAdd(devEUI);
}

void ApiRest::onActivateResponse(QJsonObject &jobj)
{
    Q_UNUSED(jobj);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString devEUI = reply->property("devEUI").toString();
    mDevManager->onDeviceActivated(devEUI);
}

void ApiRest::onGetDeviceAddress(QJsonObject &jobj)
{
    Q_UNUSED(jobj);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString devEUI = reply->property("devEUI").toString();
    QString address = jobj["devAddr"].toString();
    mDevManager->onDeviceAddress(devEUI, address);
}

void ApiRest::onDeviceMessageResponse(QJsonObject &jobj)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString devEUI = reply->property("devEUI").toString();
    gMainWindow->onDeviceMessage(devEUI, jobj);
}


