#ifndef APIREST_H
#define APIREST_H

#include <QObject>
#include <QNetworkReply>
#include <QSettings>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


class DevManager;



class ApiRest : public QObject
{
    Q_OBJECT

    DevManager* mDevManager = nullptr;

    enum struct RequestType {
        None = 0,
        GetDevices,
        DeleteDevice,
        AddDevice,
        SetDeviceKeys
    };

    QString mApiUrl, mApiKey, mAppId;
    uint mApiPort = 0;
    QNetworkAccessManager mManager;
    // QNetworkReply *mReply = NULL;

    QNetworkRequest createRequest(const QString& url, QUrlQuery query = QUrlQuery()
                                  , int limit = 0, int offset = 0);
    void prepareReply(QNetworkReply *reply, RequestType type);
    QNetworkReply *get(const QString& url, RequestType type, QUrlQuery query = QUrlQuery() , int limit = 0, int offset = 0);
    QNetworkReply *post(const QString& url, RequestType type, const QByteArray &data, QUrlQuery query = QUrlQuery() );
    QNetworkReply *del(const QString& url, RequestType type, QUrlQuery query = QUrlQuery() );


    void setDeviceKeys(const QString &devEUI, const QString &joinEUI, const QString &nwkKey);

    void onGetDevicesResponse(QJsonObject& jobj);
    void onDeleteDeviceResponse(QJsonObject& jobj);
    void onAddDeviceResponse(QJsonObject& jobj);
    void onSetDeviceKeysResponse(QJsonObject& jobj);



private slots:
    void onResponse();
    void onError(QNetworkReply::NetworkError code);

public:
    explicit ApiRest(DevManager* DevManager, const QSettings& settings);

    void getDevices(int count = 0);
    void deleteDevice(QString devEUI);
    void addDevice(const QString& name, const QString &profileId, const QString &devEUI, const QString &joinEUI, const QString &nwkKey );
};

#endif // APIREST_H
