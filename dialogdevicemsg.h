#ifndef DIALOGDEVICEMSG_H
#define DIALOGDEVICEMSG_H


#include <QDialog>

class LoraDev;
class DevManager;
class QTableWidgetItem;

namespace Ui {
class DialogDeviceMsg;
}

class DialogDeviceMsg : public QDialog
{
    Q_OBJECT

    QString getSelectedDeviceEUI();
    DevManager* mDevManager = nullptr;

    QIcon mIconLightOn, mIconLightOff;
    QIcon mIconSoundOn, mIconSoundOff;
    QIcon mIconShockOn, mIconShockOff;

    QIcon loadIcon(const QString& iconName);

    void closeEvent(QCloseEvent* e);
    QString currentEUI();

    LoraDev* deviceByRow(int row);

    typedef struct DevCon {
        int mRow;
        QMetaObject::Connection mConn;
    } DeviceConnection;

    QMap<QByteArray, DevCon> mDevsMap;

    void clearConnections();
    bool changeDeviceMsgIcon(int row, const QByteArray& msg, bool isOn);
    void onDeviceMessageAfter(const QByteArray &addr, const QByteArray& msg);
public:
    explicit DialogDeviceMsg(DevManager *devManager, QWidget *parent = nullptr);
    ~DialogDeviceMsg();

    void updateDevices();
    void onResponse(const QString &devEUI, const QJsonObject &jobjResponse);
    void onDeviceMessage(const QByteArray &addr, const QByteArray& msg);

private slots:
    void on_btnSendLight_clicked();

    void on_btnSendSound_clicked();

    void on_btnSendShock_clicked();

    void on_tableDevices_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

    void on_btnSend_clicked();

private:
    Ui::DialogDeviceMsg *ui;
};

#endif // DIALOGDEVICEMSG_H
