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
        ActivateDevice,
        GetDeviceAddress,
        SendDeviceMessage,
        GetGateways
    };

    QString mApiUrl, mApiKey, mAppId;
    QString mTenantId, mTenantKey;
    uint mApiPort = 0;
    QNetworkAccessManager mManager;
    // QNetworkReply *mReply = NULL;

    QNetworkRequest createRequest(bool isTenant, const QString& url, QUrlQuery query = QUrlQuery()
                                  , int limit = 0, int offset = 0);
    void prepareReply(QNetworkReply *reply, RequestType type);

    QNetworkReply *get( bool isTenant,
                        const QString& url,
                        RequestType type,
                        QUrlQuery query = QUrlQuery() ,
                        int limit = 0, int offset = 0);

    QNetworkReply *post(bool isTenant,
                        const QString& url,
                        RequestType type,
                        const QByteArray &data = QByteArray(),
                        QUrlQuery query = QUrlQuery() );

    QNetworkReply *del( bool isTenant,
                        const QString& url,
                        RequestType type,
                        QUrlQuery query = QUrlQuery() );



    void onGetDevicesResponse(QJsonObject& jobj);
    void onDeleteDeviceResponse(QJsonObject& jobj);
    void onAddDeviceResponse(QJsonObject& jobj);
    void onActivateResponse(QJsonObject& jobj);
    void onGetDeviceAddress(QJsonObject& jobj);
    void onDeviceMessageResponse(QJsonObject& jobj);
    void onGetGatewaysResponse(QJsonObject& jobj);


private slots:
    void onResponse();
    void onError(QNetworkReply::NetworkError code);

public:
    explicit ApiRest(const QSettings& settings, DevManager* DevManager);

    void getDevices(int count = 0);

    void deleteDevice(const QString& devEUI);
    void addDevice(const QString& name,
                   const QString &profileId,
                   const QString &devEUI);

    void getDeviceAddress(const QString& devEUI);
    void activateDevice(const QString &devEUI,
                        const QString &devAddr,
                        const QString& appSKey,
                        const QString &nwkSKey);

    void sendDeviceMessage(const QString& devEUI, const QByteArray& msg, uint8_t fPort);

    void getGateways(int count = 0);

    const QString& tenantId() { return mTenantId; }
};

#endif // APIREST_H
