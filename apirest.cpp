#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkRequest>
#include "defines_settings.h"
#include "mainwindow.h"
#include "apirest.h"


ApiRest::ApiRest(const QSettings &settings, MainWindow *mainWindow)
    : mMainWindow{mainWindow}
{
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
    mMainWindow->onError( QString("REST error: %1").arg(code) );
}

QNetworkRequest ApiRest::createRequest(const QString &url, const QUrlQuery &query)
{
    QUrl urlFull = QString( "%1:%2%3" ).arg(mApiUrl).arg(mApiPort).arg(url);
    if( !query.isEmpty() ) {
        urlFull.setQuery(query);
    }

    QNetworkRequest request(urlFull);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(mApiKey).toLatin1() );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("User-Agent", "Herdming-simulator/1.0");

    return request;
}

void ApiRest::prepareReply(RequestType type)
{
    // Too long for Lambda expression
    connect(mReply, &QNetworkReply::finished, this, &ApiRest::onResponse);
    connect(mReply, &QNetworkReply::errorOccurred, this, &ApiRest::onError );
    mReply->setProperty("requestType", static_cast<int>(type));
}


void ApiRest::get(const QString &url, RequestType type, const QUrlQuery& query)
{
    QNetworkRequest request = createRequest(url, query);
    mReply = mManager.get(request);
    prepareReply(type);
    qDebug() << "Get request:" << request.url().toString();
}

void ApiRest::post(const QString &url, RequestType type, const QByteArray& data, const QUrlQuery &query)
{
    QNetworkRequest request = createRequest(url, query);
    mReply = mManager.post(request, data);
    prepareReply(type);
    qDebug() << "Post request:" << request.url().toString();
}

void ApiRest::getDevices()
{
    QUrlQuery query;
    query.addQueryItem("applicationId", mAppId);
    get("/api/devices", RequestType::Devices, query);
}


