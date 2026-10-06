#ifndef DIALOGREGISTERANIMAL_H
#define DIALOGREGISTERANIMAL_H



#include <QDialog>
//#include <QColor>

#include "hardware/loradev_sim.h"

class Herd;
class QListWidgetItem;

namespace Ui {
class DialogRegisterAnimal;
}

class DialogRegisterAnimal : public QDialog
{
    Q_OBJECT

    QIcon mMale, mFemale, mMaleNew, mFemaleNew;

    Herd* mHerd = nullptr;
    QStringList mNames;

    void updateExisting();
    void clear();
    bool mIsChange = false;


    bool isMale();
    QString name();
    QString eui();
    bool isCollar();
    bool isBolus();
    bool isRelay();

    bool mIsSugestingNames = false;

public:
    explicit DialogRegisterAnimal(Herd *herd, QWidget *parent = nullptr);
    ~DialogRegisterAnimal();

    bool devicesChanged(){ return mIsChange; }

private slots:

    void on_btnClose_clicked();

    void on_btnAdd_clicked();

    void on_btnRemove_clicked();

    void on_btnCancel_clicked();

    void on_listRegister_itemSelectionChanged();

    void on_btnRegister_clicked();



    void on_comboName_editTextChanged(const QString &arg1);

private:
    Ui::DialogRegisterAnimal *ui;
};



#endif // DIALOGREGISTERANIMAL_H
