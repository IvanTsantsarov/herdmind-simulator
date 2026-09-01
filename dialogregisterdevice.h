#ifndef DIALOGREGISTERDEVICE_H
#define DIALOGREGISTERDEVICE_H

#include <QDialog>

namespace Ui {
class DialogRegisterDevice;
}

class DialogRegisterDevice : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRegisterDevice(QWidget *parent = nullptr);
    ~DialogRegisterDevice();

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

private:
    Ui::DialogRegisterDevice *ui;
};

#endif // DIALOGREGISTERDEVICE_H
