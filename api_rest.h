#ifndef API_REST_H
#define API_REST_H

#include <QObject>
#include <QNetworkReply>
#include <QUrlQuery>

class MainWindow;


class ApiRest : public QObject
{
    Q_OBJECT

    QNetworkAccessManager mManager;
    MainWindow* mMainWindow = NULL;
    QNetworkReply *mReply = NULL;

    void get(const QString& url, const QUrlQuery& query = QUrlQuery() );
public:
    explicit ApiRest(MainWindow *mainWindow = nullptr);

    void getPairs();

private slots:
    void onReplyFinished();
    void onError(QNetworkReply::NetworkError code);
};

#endif // API_REST_H
