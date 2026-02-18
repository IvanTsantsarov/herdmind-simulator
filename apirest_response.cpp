#include "apirest.h"
#include "mainwindow.h"
#include "qjsonparseerror.h"

void ApiRest::onResponse()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "REST:Request failed:" << reply->errorString() << responseData;
        mMainWindow->setStatus( "REST:Network failure!" );
        return;
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError && jsonDoc.isObject()) {
        qDebug() << "REST:Failed to parse JSON:" << parseError.errorString();
        mMainWindow->setStatus( "REST:JSON parse error!" );
        return;
    }

    auto json = jsonDoc.object();
    qDebug() << QJsonDocument(json).toJson(QJsonDocument::Indented); // Uncomment for Debug

    RequestType type = static_cast<RequestType>( reply->property("requestType").toInt() );

    switch(type) {
        case RequestType::None: break;
        case RequestType::GetDevices: onGetDevicesResponse(json); break;
        case RequestType::GetApplications: onGetDevicesResponse(json); break;
        }
}

void ApiRest::onGetDevicesResponse(QJsonObject& jobj)
{
    /*
     * QJsonArray data = jobj["data"].toArray();
    // Make pairs map
    Pairs pairs;
    foreach( QJsonValue val, data) {
        QString pair = val["instFamily"].toString();

        // if pair key not exist - create a new list
        if( !pairs.contains(pair) ) {
            pairs[pair] = QStringList();
        }

        // append instrument to the list
        pairs[pair].append(val["instId"].toString());
    }

    mMainWindow->onPairs( pairs );
*/
}

void ApiRest::onGetApplicationsResponse(QJsonObject &jobj)
{

}

