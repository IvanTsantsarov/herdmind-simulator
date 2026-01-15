#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "mainwindow.h"
#include "defines.h"
#include "api_rest.h"


ApiRest::ApiRest(MainWindow *mainWindow)
    : mMainWindow{mainWindow}
{

}

/*
curl -X 'POST' \
    'https://www.okx.com/api/v5/trade/order' \
    -H 'accept: application/json' \
    -H 'Content-Type: application/json' \
    -H 'OK-ACCESS-KEY: *****' \
    -H 'OK-ACCESS-SIGN: *****'' \
    -H 'OK-ACCESS-TIMESTAMP: *****'' \
    -H 'OK-ACCESS-PASSPHRASE: *****'' \
    -H 'expTime: 1597026383085' \   // request effective deadline
    -d '{
  "instId": "BTC-USDT",
    "tdMode": "cash",
               "side": "buy",
                        "ordType": "limit",
                                    "px": "1000",
                                           "sz": "0.01"
}'
*/

void ApiRest::get(const QString &url, const QUrlQuery& query)
{

    QUrl urlFull = REST_URL + url;
    if( !query.isEmpty() ) {
        urlFull.setQuery(query);
    }

    QNetworkRequest request(urlFull);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "DiaDrawTest/1.0");

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
}



void ApiRest::getPairs()
{
    QUrlQuery query;
    query.addQueryItem( "instType", "FUTURES" );
    get( REST_GET_INSTRUMENTS, query );
}

