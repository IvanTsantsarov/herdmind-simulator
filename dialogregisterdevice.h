#ifndef DIALOGREGISTERDEVICE_H
#define DIALOGREGISTERDEVICE_H



#include <QDialog>
//#include <QColor>

#include "hardware/loradev.h"

class Herd;

namespace Ui {
class DialogRegisterDevice;
}

class DialogRegisterDevice : public QDialog
{
    Q_OBJECT

    Herd* mHerd = nullptr;
    QStringList mNames;
    QColor mStatusBackColor;

    void updateExisting();
    void clear();
    void setStatus(const QString& txt, const QColor &col);
    bool mIsChange = false;

    bool isNew();
    QString name();
    LoraDev::Profile profile();
    bool isCollar();
    bool isBolus();
    bool isRelay();

public:
    explicit DialogRegisterDevice(Herd *herd, QWidget *parent = nullptr);
    ~DialogRegisterDevice();

    bool devicesChanged(){ return mIsChange; }

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

    void on_btnClear_clicked();

    void on_btnRegister_clicked();

private:
    Ui::DialogRegisterDevice *ui;
};



#endif // DIALOGREGISTERDEVICE_H
