#ifndef APIREST_H
#define APIREST_H

#include <QObject>
#include <QNetworkReply>
#include <QSettings>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


class Storage;


class ApiRest : public QObject
{
    Q_OBJECT

    Storage* mStorage = nullptr;

    enum struct RequestType {
        None = 0,
        GetDevices
    };

    QString mApiUrl, mApiKey, mAppId;
    uint mApiPort = 0;
    QNetworkAccessManager mManager;
    // QNetworkReply *mReply = NULL;

    QNetworkRequest createRequest(const QString& url, QUrlQuery query = QUrlQuery()
                                  , int limit = 0, int offset = 0);
    void prepareReply(QNetworkReply *reply, RequestType type);
    QNetworkReply *get(const QString& url, RequestType type, QUrlQuery query = QUrlQuery() , int limit = 0, int offset = 0);
    QNetworkReply *post(const QString& url, RequestType type, const QByteArray &data = QByteArray(), QUrlQuery query = QUrlQuery() );
    QNetworkReply *del(const QString& url, RequestType type, QUrlQuery query = QUrlQuery() );



    void onGetDevicesResponse(QJsonObject& jobj);
    void onDeleteDeviceResponse(QJsonObject& jobj);
    void onAddDeviceResponse(QJsonObject& jobj);
    void onActivateResponse(QJsonObject& jobj);
    void onGetDeviceAddress(QJsonObject& jobj);
    void onDeviceMessageResponse(QJsonObject& jobj);


private slots:
    void onResponse();
    void onError(QNetworkReply::NetworkError code);

public:
    explicit ApiRest(Storage* st, const QSettings& settings);

    void getDevices(int count = 0);

    void sendDeviceMessage(const QString& devEUI, const QByteArray& msg);

};

#endif // APIREST_H
