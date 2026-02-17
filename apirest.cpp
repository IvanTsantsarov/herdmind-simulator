#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "defines_settings.h"
#include "mainwindow.h"
#include "apirest.h"


ApiRest::ApiRest(const QSettings &settings, MainWindow *mainWindow)
    : mMainWindow{mainWindow}
{
    mApiUrl = settings.value( CHIRPSTACK_SECTION"/apiUrl").toString();
    mApiKey = settings.value( CHIRPSTACK_SECTION"/apiKey").toString();
    mApiPort = settings.value( CHIRPSTACK_SECTION"/apiPort").toUInt();
}

/*
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

void ApiRest::get(const QString &url, const QUrlQuery& query)
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


    // Send GET request
    mReply = mManager.get(request);

    // Too long for Lambda expression
    connect(mReply, &QNetworkReply::finished, this, &ApiRest::onReplyFinished);
    connect(mReply, &QNetworkReply::errorOccurred, this, &ApiRest::onError );

    qDebug() << "Get request:" << request.url().toString();
}

void ApiRest::onError(QNetworkReply::NetworkError code)
{
    mMainWindow->onError( QString("REST error: %1").arg(code) );
}

void ApiRest::onReplyFinished()
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
    // qDebug() << QJsonDocument(json).toJson(QJsonDocument::Indented); // Uncomment for Debug

    QJsonArray data = json["data"].toArray();

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



void ApiRest::getPairs()
{
    QUrlQuery query;
    query.addQueryItem( "instType", "FUTURES" );
    // get( REST_GET_INSTRUMENTS, query );
}

void ApiRest::getDevices()
{
    QUrlQuery query;
    get("/api/devices",query);
}

