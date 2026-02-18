#ifndef APIREST_H
#define APIREST_H

#include <QObject>
#include <QNetworkReply>
#include <QSettings>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


class MainWindow;


class ApiRest : public QObject
{
    Q_OBJECT

    enum struct RequestType {
        None = 0,
        Devices = 1
    };

    QString mApiUrl, mApiKey, mAppId;
    uint mApiPort = 0;
    QNetworkAccessManager mManager;
    MainWindow* mMainWindow = NULL;
    QNetworkReply *mReply = NULL;

    QNetworkRequest createRequest( const QString& url, const QUrlQuery& query = QUrlQuery() );
    void prepareReply(RequestType type);
    void get(const QString& url, RequestType type, const QUrlQuery& query = QUrlQuery() );
    void post(const QString& url, RequestType type, const QByteArray &data, const QUrlQuery& query = QUrlQuery() );

    void onDevicesResponse(QJsonObject& jobj);

public:
    explicit ApiRest(const QSettings& settings, MainWindow *mainWindow = nullptr);

    void getDevices();

private slots:
    void onResponse();
    void onError(QNetworkReply::NetworkError code);
};

#endif // APIREST_H
