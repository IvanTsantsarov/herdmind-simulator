#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkRequest>
#include "defines.h"
#include "apirest.h"
#include "storage.h"

ApiRest::ApiRest(Storage *st, const QSettings &settings)
{
    mStorage = st;
    mApiUrl = settings.value( CHIRPSTACK_SECTION"/apiUrl").toString();
    mApiKey = settings.value( CHIRPSTACK_SECTION"/apiKey").toString();
    mAppId = settings.value( CHIRPSTACK_SECTION"/appId").toString();
    mApiPort = settings.value( CHIRPSTACK_SECTION"/apiPort").toUInt();
}

void ApiRest::onError(QNetworkReply::NetworkError code)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    qFatal() << QString("REST Api error: %1 '%2' - %3")
                       .arg(code)
                       .arg(reply->errorString())
                       .arg("Is it Chirpstack server running?");
}

QNetworkRequest ApiRest::createRequest(const QString &url, QUrlQuery query,
                                       int limit, int offset)
{
    QUrl urlFull = QString( "%1:%2/api/%3" ).arg(mApiUrl).arg(mApiPort).arg(url);

    if( offset || limit ) {
        query.addQueryItem("limit", QString("%1").arg(limit) );
        query.addQueryItem("offset", QString("%1").arg(offset) );
    }
    urlFull.setQuery(query);

    QNetworkRequest request(urlFull);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(mApiKey).toLatin1() );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("User-Agent", "Herdming-simulator/1.0");

    return request;
}

void ApiRest::prepareReply(QNetworkReply* reply, RequestType type)
{
    // Too long for Lambda expression
    connect(reply, &QNetworkReply::finished, this, &ApiRest::onResponse);
    connect(reply, &QNetworkReply::errorOccurred, this, &ApiRest::onError );
    reply->setProperty("requestType", static_cast<int>(type));
}


QNetworkReply* ApiRest::get(const QString &url, RequestType type, QUrlQuery query, int limit, int offset)
{
    QNetworkRequest request = createRequest(url, query, limit, offset);
    QNetworkReply* reply = mManager.get(request);
    prepareReply(reply, type);
    // qDebug() << "Get request:" << request.url().toString() << query.toString();
    return reply;
}

QNetworkReply* ApiRest::post(const QString &url,
                             RequestType type,
                             const QByteArray& data,
                             QUrlQuery query)
{
    QNetworkRequest request = createRequest(url, query);
    QNetworkReply* reply = mManager.post(request, data);
    prepareReply(reply, type);
    // qDebug() << "Post request:" << request.url().toString() << query.toString() << data;
    return reply;
}

QNetworkReply* ApiRest::del(const QString &url, RequestType type, QUrlQuery query)
{
    QNetworkRequest request = createRequest(url, query);
    QNetworkReply* reply = mManager.deleteResource(request);
    prepareReply(reply, type);
    // qDebug() << "Delete request:" << request.url().toString() << query.toString();
    return reply;
}



void ApiRest::getDevices(int count)
{
    QUrlQuery query;
    query.addQueryItem("applicationId", mAppId);
    get("devices", RequestType::GetDevices, query, count);
}


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
    }
}

void ApiRest::onGetDevicesResponse(QJsonObject& jobj)
{
    mStorage->registerDevices(jobj);
}
