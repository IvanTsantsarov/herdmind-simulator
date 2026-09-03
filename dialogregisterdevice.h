#ifndef DIALOGREGISTERDEVICE_H
#define DIALOGREGISTERDEVICE_H

#include <QDialog>

class Herd;

namespace Ui {
class DialogRegisterDevice;
}

class DialogRegisterDevice : public QDialog
{
    Q_OBJECT

    QStringList mHerdNames;
    void updateExisting();

public:
    explicit DialogRegisterDevice(QStringList animals, QWidget *parent = nullptr);
    ~DialogRegisterDevice();

    bool isVirtual();
    bool isNew();
    QString name();




private slots:
    void on_checkVirtual_toggled(bool checked);

    void on_btnClose_clicked();

    void on_btnGenEUI_clicked();

    void on_btnGenAKey_clicked();

    void on_btnGenNKey_clicked();

    void on_btnClearEUI_clicked();

    void on_btnClearAKey_clicked();

    void on_btnClearSKey_clicked();

    void on_btnCopyEUI_clicked();

    void on_btnCopyASKey_clicked();

    void on_btnCopyNSKey_clicked();

    void on_radioAnimalNew_toggled(bool checked);

private:
    Ui::DialogRegisterDevice *ui;
};

#endif // DIALOGREGISTERDEVICE_H
