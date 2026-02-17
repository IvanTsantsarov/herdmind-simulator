#ifndef APIREST_H
#define APIREST_H

#include <QObject>
#include <QNetworkReply>
#include <QSettings>
#include <QUrlQuery>

class MainWindow;


class ApiRest : public QObject
{
    Q_OBJECT

    QString mApiUrl, mApiKey;
    uint mApiPort = 0;
    QNetworkAccessManager mManager;
    MainWindow* mMainWindow = NULL;
    QNetworkReply *mReply = NULL;

    void get(const QString& url, const QUrlQuery& query = QUrlQuery() );
public:
    explicit ApiRest(const QSettings& settings, MainWindow *mainWindow = nullptr);

    void getPairs();

    void getDevices();

private slots:
    void onReplyFinished();
    void onError(QNetworkReply::NetworkError code);
};

#endif // APIREST_H
