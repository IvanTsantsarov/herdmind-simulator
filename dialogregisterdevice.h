#ifndef DIALOGREGISTERDEVICE_H
#define DIALOGREGISTERDEVICE_H



#include <QDialog>
//#include <QColor>

#include "hardware/loradev.h"

class Herd;
class QListWidgetItem;

namespace Ui {
class DialogRegisterDevice;
}

class DialogRegisterDevice : public QDialog
{
    Q_OBJECT

    struct Record {
        QString mName;
        bool mIsNew = false;
        bool mIsMale = false;
        LoraDev::Profile mProfile;
        QString mEui;
        QListWidgetItem* item = nullptr;
    };

    QList<Record> mRecords;

    QIcon mMale, mFemale, mMaleNew, mFemaleNew;

    Herd* mHerd = nullptr;
    QStringList mNames;

    void updateExisting();
    void clear();
    bool mIsChange = false;

    bool isNew();
    bool isMale();
    QString name();
    QString eui();
    LoraDev::Profile profile();
    bool isCollar();
    bool isBolus();
    bool isRelay();

public:
    explicit DialogRegisterDevice(Herd *herd, QWidget *parent = nullptr);
    ~DialogRegisterDevice();

    bool devicesChanged(){ return mIsChange; }

private slots:

    void on_btnClose_clicked();

    void on_btnGenEUI_clicked();

    void on_btnClearEUI_clicked();

    void on_btnCopyEUI_clicked();

    void on_radioAnimalNew_toggled(bool checked);

    void on_btnRegister_clicked();

    void on_btnAdd_clicked();

    void on_btnRemove_clicked();

    void on_btnCancel_clicked();

private:
    Ui::DialogRegisterDevice *ui;
};



#endif // DIALOGREGISTERDEVICE_H
