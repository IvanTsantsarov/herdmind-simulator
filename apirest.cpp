#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkRequest>
#include "defines_settings.h"
#include "mainwindow.h"
#include "apirest.h"
#include "simtools.h"

#define CHECK_EMPTY_SETTINGS_VALUE(__value__, __err_str__) __value__

ApiRest::ApiRest(DevManager *DevManager, const QSettings &settings)
{
    mDevManager = DevManager;
    mApiUrl = SimTools::readStringSettingsValue(settings, CHIRPSTACK_SECTION, "apiUrl");
    mApiKey = SimTools::readStringSettingsValue(settings, CHIRPSTACK_SECTION, "apiKey");
    mAppId = SimTools::readStringSettingsValue(settings, CHIRPSTACK_SECTION, "appId");
    mApiPort = SimTools::readIntSettingsValue(settings, CHIRPSTACK_SECTION, "apiPort");
    mTenantKey = SimTools::readStringSettingsValue(settings, CHIRPSTACK_SECTION, "tenantKey");
    mTenantId =SimTools::readStringSettingsValue(settings, CHIRPSTACK_SECTION, "tenantId");
}

void ApiRest::onError(QNetworkReply::NetworkError code)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    qCritical() << QString("REST Api error: %1 '%2': %3")
                       .arg(code)
                       .arg(reply->errorString())
                       .arg(reply->readAll())
                << reply->request().url().toString();

}

QNetworkRequest ApiRest::createRequest(bool isTenant, const QString &url, QUrlQuery query,
                                       int limit, int offset)
{
    QUrl urlFull = QString( "%1:%2/api/%3" ).arg(mApiUrl).arg(mApiPort).arg(url);

    if( offset || limit ) {
        query.addQueryItem("limit", QString("%1").arg(limit) );
        query.addQueryItem("offset", QString("%1").arg(offset) );
    }
    urlFull.setQuery(query);

    QNetworkRequest request(urlFull);

    request.setRawHeader("Authorization", QString("Bearer %1").arg(isTenant? mTenantKey : mApiKey).toLatin1() );
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


QNetworkReply* ApiRest::get(bool isTenant, const QString &url, RequestType type, QUrlQuery query, int limit, int offset)
{
    QNetworkRequest request = createRequest(isTenant, url, query, limit, offset);
    QNetworkReply* reply = mManager.get(request);
    prepareReply(reply, type);
    qDebug() << "Get request:" << request.url().toString() << query.toString();
    return reply;
}

QNetworkReply* ApiRest::post(bool isTenant, const QString &url,
                             RequestType type,
                             const QByteArray& data,
                             QUrlQuery query)
{
    QNetworkRequest request = createRequest(isTenant, url, query);
    QNetworkReply* reply = mManager.post(request, data);
    prepareReply(reply, type);
    qDebug() << "Post request:" << request.url().toString() << query.toString() << data;
    return reply;
}

QNetworkReply* ApiRest::del(bool isTenant, const QString &url, RequestType type, QUrlQuery query)
{
    QNetworkRequest request = createRequest(isTenant, url, query);
    QNetworkReply* reply = mManager.deleteResource(request);
    prepareReply(reply, type);
    qDebug() << "Delete request:" << request.url().toString() << query.toString();
    return reply;
}

void ApiRest::addDevice(const QString& name,
                        const QString &profileId,
                        const QString &devEUI )
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

    QNetworkReply* reply = post(false, "devices", RequestType::AddDevice, data.toUtf8() );
    reply->setProperty("devEUI", devEUI);
}


// Activation By Personalization (static)
void ApiRest::activateDevice(const QString &devEUI,
                             const QString &devAddr,
                             const QString &appSKey,
                             const QString &nwkSKey)
{
    QString data = QString(
                       "{"
                       "\"deviceActivation\": {"
                       "\"devEui\": \"%1\","
                       "\"devAddr\": \"%2\","
                       "\"appSKey\": \"%3\","
                       "\"nwkSEncKey\": \"%4\","
                       "\"sNwkSIntKey\": \"%4\","
                       "\"fNwkSIntKey\": \"%4\","
                       "\"fCntUp\": 0,"
                       "\"nFCntDown\": 0,"
                       "\"aFCntDown\": 0"
                       "}}" )
                       .arg(devEUI)
                       .arg(devAddr)
                       .arg(appSKey)
                       .arg(nwkSKey);

    QNetworkReply* reply = post(false,
                                QString("devices/%1/activate").arg(devEUI),
                                RequestType::ActivateDevice, data.toUtf8() );
    reply->setProperty("devEUI", devEUI);
}

void ApiRest::sendDeviceMessage(const QString &devEUI, const QByteArray &msg, uint8_t fPort)
{
    QString data = QString(
    "{\"queueItem\": {"
        "\"confirmed\": false,"
        "\"data\": \"%1\","
        "\"fPort\": %2"
        "} }")
    .arg(msg.toBase64())
    .arg(fPort);

    QNetworkReply* reply = post(false,
                                QString("devices/%1/queue").arg(devEUI),
                                RequestType::SendDeviceMessage, data.toUtf8() );
    reply->setProperty("devEUI", devEUI);
}

void ApiRest::getDevices(int count)
{
    QUrlQuery query;
    query.addQueryItem("applicationId", mAppId);
    get(false, "devices", RequestType::GetDevices, query, count);
}

void ApiRest::getDeviceAddress(const QString &devEUI)
{
    QNetworkReply* reply = post(false, QString("devices/%1/get-random-dev-addr").arg(devEUI),
                                RequestType::GetDeviceAddress );
    reply->setProperty("devEUI", devEUI);
}

void ApiRest::deleteDevice(const QString &devEUI)
{
    QNetworkReply* reply = del( false, QString("devices/%1").arg(devEUI), RequestType::DeleteDevice );
    reply->setProperty("devEUI", devEUI);
}


void ApiRest::getGateways(int count)
{
    QUrlQuery query;
    query.addQueryItem("tenantId", mTenantId);

    QNetworkReply* reply = get(true, "gateways",
                               RequestType::GetGateways,
                               query, count);
    reply->setProperty("tenantId", mTenantId);
}

