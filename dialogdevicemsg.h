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
public:
    explicit DialogDeviceMsg(DevManager *devManager, QWidget *parent = nullptr);
    ~DialogDeviceMsg();

    void updateDevices();
    void onResponse(const QString &devEUI, const QJsonObject &jobjResponse);

private slots:
    void on_btnSendLight_clicked();

    void on_btnSendSound_clicked();

    void on_btnSendShock_clicked();

    void on_tableDevices_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

private:
    Ui::DialogDeviceMsg *ui;
};

#endif // DIALOGDEVICEMSG_H
