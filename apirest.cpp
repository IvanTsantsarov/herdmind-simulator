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

/*

curl -X GET "http://localhost:8090/api/devices?applicationId=fc82a77f-d448-43ac-a381-7289d4e5ba2d&limit=100&offset=0" \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjgwNGFjZDQ4LTFhNjAtNGNhYi1iYjRkLTQ3Mzc0YjYxZDg2YSIsInR5cCI6ImtleSJ9.cFLwX6uOmSrugrEovaLBpF3H7izuqM8Awdp-9Vx8Aqg" \
  -H "Accept: application/json"

 *
Adding devices to ChirpStack via REST API involves authenticating with an API key, then sending a POST request to the /api/devices endpoint with the device details (DevEUI, Application ID, Device Profile ID) in JSON format. The device must be associated with an existing application and a device profile.
Steps to Add a Device via REST API:
Generate an API Key: Create a global API key in the ChirpStack web interface to authorize your REST calls.
Identify Required IDs: Obtain the applicationID and deviceProfileID (found in the URL or details page of those entities in the UI).
Construct POST Request: Use the {Link: ChirpStack REST API v4.14.1 - Swagger UI - MeteoScientific https://console.meteoscientific.com/rest-api/} (or your specific version's API documentation) to send a request.
API Request Example (POST /api/devices):
json
POST /api/devices
Authorization: Bearer <YOUR_API_KEY>

{
  "device": {
    "devEui": "0000000000000000",
    "name": "my-device",
    "applicationId": "your-application-id",
    "deviceProfileId": "your-device-profile-id",
    "description": "Optional description",
    "isDisabled": false,
    "skipFcntCheck": false,
    "referenceAltitude": 0
  }
}
Key Notes:
Ensure the deviceProfileId is correct to avoid errors.
If using OTAA, you will need to perform a second step to set the root keys (AppKey) using POST /api/devices/{devEui}/keys.
For bulk operations, scripting these POST requests is recommended. */

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
    qInfo() << "Get request:" << request.url().toString();
    return reply;
}

QNetworkReply* ApiRest::post(const QString &url, RequestType type, const QByteArray& data, QUrlQuery query)
{
    QNetworkRequest request = createRequest(url, query);
    QNetworkReply* reply = mManager.post(request, data);
    prepareReply(reply, type);
    qInfo() << "Post request:" << request.url().toString();
    return reply;
}

QNetworkReply* ApiRest::del(const QString &url, RequestType type, QUrlQuery query)
{
    QNetworkRequest request = createRequest(url, query);
    QNetworkReply* reply = mManager.deleteResource(request);
    prepareReply(reply, type);
    qInfo() << "Delete request:" << request.url().toString();
    return reply;
}

void ApiRest::addDevice(const QString& name,
                        const QByteArray &profileId,
                        const QByteArray &devEUI,
                        const QByteArray &joinEUI,
                        const QByteArray &nwkKey )
{
    QString data = QString(
                       "{"
                       "\"device\": {"
                       "\"applicationId\": \"%1\","
                       "\"devEui\": \"%2\","
                       "\"name\": \"%3\","
                       "\"deviceProfileId\": \"%4\""
                       "}"
                       "}")
                       .arg(gSimTools->appId().toHex())
                       .arg(devEUI.toHex())
                       .arg(name.toUtf8())
                       .arg(profileId.toHex());

    QNetworkReply* reply = post("devices", RequestType::AddDevice, data.toUtf8() );
    reply->setProperty("devEUI", devEUI);
    reply->setProperty("joinEUI", joinEUI);
    reply->setProperty("nwkKey", nwkKey);
}

void ApiRest::setDeviceKeys(const QByteArray &devEUI, const QByteArray &joinEUI, const QByteArray &nwkKey)
{
    QByteArray devHex = devEUI.toHex();
    QString data = QString(
                       "{"
                       "\"deviceKeys\": {"
                       "\"devEui\": \"%1\","
                       "\"joinEui\": \"%2\","
                       "\"nkwKey\": \"%3\","
                       "}"
                       "}")
                       .arg(devHex)
                       .arg(joinEUI.toHex())
                       .arg(nwkKey.toHex());

    post(QString("/api/devices/%1/keys").arg(devHex), RequestType::AddDevice, data.toUtf8() );
}

void ApiRest::getDevices(int count)
{
    QUrlQuery query;
    query.addQueryItem("applicationId", mAppId);
    get("devices", RequestType::GetDevices, query, count);
}

void ApiRest::deleteDevice(QString devEUI)
{
    del( QString("devices/%1").arg(devEUI), RequestType::DeleteDevice );
}


