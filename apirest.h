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
        GetDevices,
        GetApplications
    };

    QString mApiUrl, mApiKey, mAppId;
    uint mApiPort = 0;
    QNetworkAccessManager mManager;
    MainWindow* mMainWindow = NULL;
    // QNetworkReply *mReply = NULL;

    QNetworkRequest createRequest( const QString& url, const QUrlQuery& query = QUrlQuery() );
    void prepareReply(QNetworkReply *reply, RequestType type);
    void get(const QString& url, RequestType type, const QUrlQuery& query = QUrlQuery() );
    void post(const QString& url, RequestType type, const QByteArray &data, const QUrlQuery& query = QUrlQuery() );

    void onGetDevicesResponse(QJsonObject& jobj);
    void onGetApplicationsResponse(QJsonObject& jobj);

public:
    explicit ApiRest(const QSettings& settings, MainWindow *mainWindow = nullptr);

    void getDevices();
    void getApplications();

private slots:
    void onResponse();
    void onError(QNetworkReply::NetworkError code);
};

#endif // APIREST_H
