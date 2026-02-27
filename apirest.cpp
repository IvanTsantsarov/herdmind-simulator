#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkRequest>
#include "defines_settings.h"
#include "mainwindow.h"
#include "apirest.h"
#include "simtools.h"


ApiRest::ApiRest(DevManager *DevManager, const QSettings &settings)
{
    mDevManager = DevManager;
    mApiUrl = settings.value( CHIRPSTACK_SECTION"/apiUrl").toString();
    mApiKey = settings.value( CHIRPSTACK_SECTION"/apiKey").toString();
    mAppId = settings.value( CHIRPSTACK_SECTION"/appId").toString();
    mApiPort = settings.value( CHIRPSTACK_SECTION"/apiPort").toUInt();
}

void ApiRest::onError(QNetworkReply::NetworkError code)
{
    qCritical() << QString("REST error: %1").arg(code);
    // mMainWindow->onError( QString("REST error: %1").arg(code) );
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
    qDebug() << "Get request:" << request.url().toString() << query.toString();
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
    qDebug() << "Post request:" << request.url().toString() << query.toString() << data;
    return reply;
}

QNetworkReply* ApiRest::del(const QString &url, RequestType type, QUrlQuery query)
{
    QNetworkRequest request = createRequest(url, query);
    QNetworkReply* reply = mManager.deleteResource(request);
    prepareReply(reply, type);
    qDebug() << "Delete request:" << request.url().toString() << query.toString();
    return reply;
}

void ApiRest::addDevice(const QString& name,
                        const QString &profileId,
                        const QString &devEUI,
                        const QString &appKey )
{
    QString data = QString(
                       "{"
                       "\"device\": {"
                       "\"applicationId\": \"%1\","
                       "\"devEui\": \"%2\","
                       "\"name\": \"%3\","
                       "\"deviceProfileId\": \"%4\","
                       "\"skipFcntCheck\": true,"
                       "\"isDisabled\": false"
                       "}"
                       "}")
                       .arg(gSimTools->appId())
                       .arg(devEUI)
                       .arg(name.toUtf8())
                       .arg(profileId);

    QNetworkReply* reply = post("devices", RequestType::AddDevice, data.toUtf8() );
    reply->setProperty("devEUI", devEUI);
    reply->setProperty("appKey", appKey);

}


// Activation By Personalization (static)
void ApiRest::activateDevice(const QString &devEUI, const QString &devAddr, const QString &appSKey)
{
    QString data = QString(
                       "{"
                       "\"deviceActivation\": {"
                       "\"devEui\": \"%1\","
                       "\"devAddr\": \"%2\","
                       "\"appSKey\": \"%3\","
                       "\"nwkSEncKey\": \"%3\","
                       "\"sNwkSIntKey\": \"%3\","
                       "\"fNwkSIntKey\": \"%3\","
                       "\"fCntUp\": 0,"
                       "\"nFCntDown\": 0,"
                       "\"aFCntDown\": 0"
                       "}}" )
                       .arg(devEUI)
                       .arg(devAddr)
                       .arg(appSKey);

    QNetworkReply* reply = post(QString("devices/%1/activate").arg(devEUI),
                                RequestType::ActivateDevice, data.toUtf8() );
    reply->setProperty("devEUI", devEUI);
}

void ApiRest::sendDeviceMessage(const QString &devEUI, const QByteArray &msg)
{
    QString data = QString(
    "{\"queueItem\": {"
        "\"confirmed\": false,"
        "\"data\": \"%1\","
        "\"fPort\": %2"
        "} }")
    .arg(msg.toBase64())
    .arg(LORA_FPORT);

    QNetworkReply* reply = post(QString("devices/%1/queue").arg(devEUI),
                                RequestType::SendDeviceMessage, data.toUtf8() );
    reply->setProperty("devEUI", devEUI);
}

void ApiRest::getDevices(int count)
{
    QUrlQuery query;
    query.addQueryItem("applicationId", mAppId);
    get("devices", RequestType::GetDevices, query, count);
}

void ApiRest::getDeviceAddress(const QString &devEUI)
{
    QNetworkReply* reply = post(QString("devices/%1/get-random-dev-addr").arg(devEUI),
                                RequestType::GetDeviceAddress );
    reply->setProperty("devEUI", devEUI);
}

void ApiRest::deleteDevice(const QString &devEUI)
{
    QNetworkReply* reply = del( QString("devices/%1").arg(devEUI), RequestType::DeleteDevice );
    reply->setProperty("devEUI", devEUI);
}


