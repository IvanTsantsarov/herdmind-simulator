#include "apirest.h"
#include "mainwindow.h"
#include "qjsonparseerror.h"

void ApiRest::onResponse()
{
    if (mReply->error() != QNetworkReply::NoError) {
        qDebug() << "REST:Request failed:" << mReply->errorString();
        mMainWindow->setStatus( "REST:Network failure!" );
        mReply->deleteLater();
        return;
    }

    QByteArray responseData = mReply->readAll();
    mReply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError && jsonDoc.isObject()) {
        qDebug() << "REST:Failed to parse JSON:" << parseError.errorString();
        mMainWindow->setStatus( "REST:JSON parse error!" );
        return;
    }

    auto json = jsonDoc.object();
    qDebug() << QJsonDocument(json).toJson(QJsonDocument::Indented); // Uncomment for Debug

    RequestType type = static_cast<RequestType>( mReply->property("requestType").toInt() );

    switch(type) {
        case RequestType::None: break;
        case RequestType::Devices: onDevicesResponse(json); break;
        }
}

void ApiRest::onDevicesResponse(QJsonObject& jobj)
{
    QJsonArray data = jobj["data"].toArray();

    /*
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

