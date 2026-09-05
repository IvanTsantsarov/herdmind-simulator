#include "dialogregisterdevice.h"
#include "ui_dialogregisterdevice.h"


#include "hardware/loradev.h"
//#include "hardware/loradev_def.h"
//#include "hardware/gateway/gateway.h"
//#include "hardware/hardware/tools.h"
#include "simtools.h"
#include "herd.h"
#include "animal.h"
#include <QToolTip>
#include <QMessageBox>

#define COL_ERR QColor(255, 50, 50)
#define COL_DONE QColor(50, 255, 50)

void DialogRegisterDevice::updateExisting()
{
    if( ui->comboHerd->count() < 1 ) {
        ui->radioAnimalExisting->setEnabled(false);
        ui->radioAnimalExisting->setChecked(false);
        ui->radioAnimalNew->setChecked(true);
        ui->comboHerd->setVisible(false);
    }else {
        ui->radioAnimalExisting->setEnabled(true);
    }
}

void DialogRegisterDevice::clear()
{
    ui->editName->clear();
    // setStatus("", mStatusBackColor);
}


DialogRegisterDevice::DialogRegisterDevice(Herd* herd, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogRegisterDevice)
{
    ui->setupUi(this);
    mHerd = herd;

    mNames = mHerd->names();

    mMale = QIcon("://male.svg");
    mFemale = QIcon("://female.svg");
    mMaleNew = QIcon("://male_new.svg");
    mFemaleNew = QIcon("://female_new.svg");

    for( const QString& name: mNames) {
        ui->comboHerd->addItem(name);
    }

    updateExisting();
}

DialogRegisterDevice::~DialogRegisterDevice()
{
    delete ui;
}

bool DialogRegisterDevice::isNew()
{
    return ui->radioAnimalNew->isChecked();
}

bool DialogRegisterDevice::isMale()
{
    return ui->radioMale->isChecked();
}

LoraDev::Profile DialogRegisterDevice::profile()
{
    if( ui->radioTypeCollar->isChecked() ) {
        return LoraDev::Profile::Collar;
    }

    if( ui->radioTypeBolus->isChecked() ) {
        return LoraDev::Profile::Bolus;
    }

    return LoraDev::Profile::None;
}

void DialogRegisterDevice::on_btnClose_clicked()
{
    close();
}


void DialogRegisterDevice::on_btnGenEUI_clicked()
{
    QByteArray ba = SimTools::genHex(EUI_BYTES_LEN);
    ui->editEUI->setText(ba.toUpper());
}




void DialogRegisterDevice::on_btnClearEUI_clicked()
{
    ui->editEUI->clear();
}


void DialogRegisterDevice::on_btnCopyEUI_clicked()
{
    SimTools::clipboardCopy(ui->editEUI->text());
    QToolTip::showText( QCursor::pos(), "EUI copied!");
}

void DialogRegisterDevice::on_radioAnimalNew_toggled(bool checked)
{
    ui->editName->setVisible(checked);
    ui->radioFemale->setVisible(checked);
    ui->radioMale->setVisible(checked);

    ui->comboHerd->setVisible(!checked);
}


bool DialogRegisterDevice::isCollar()
{
    return ui->radioTypeCollar->isChecked();
}

bool DialogRegisterDevice::isBolus()
{
    return ui->radioTypeBolus->isChecked();
}

bool DialogRegisterDevice::isRelay()
{
    return ui->radioTypeRelay->isChecked();
}


QString DialogRegisterDevice::name()
{
    return ui->editName->text();
}

QString DialogRegisterDevice::eui()
{
    return ui->editEUI->text();
}

void DialogRegisterDevice::on_btnAdd_clicked()
{
    if( name().isEmpty()) {
        ui->editName->setFocus();
        return;
    }

    if( eui().isEmpty()) {
        ui->editEUI->setFocus();
        return;
    }

    Record r;
    r.mIsNew = isNew();
    r.mName = name();
    r.mEui = eui();
    r.mProfile = profile();
    r.mIsMale = isMale();
    QString row = QString("%1 %2 %3")
        .arg(r.mIsNew? "*": "")
        .arg(r.mName)
        .arg(r.mEui);


    QIcon icon;
    if( isMale() ) {
        icon = isNew() ? mMaleNew : mMale;
    }else {
        icon = isNew() ? mFemaleNew : mFemale;
    }

    QListWidgetItem* item = new QListWidgetItem( icon, row, ui->listRegister );
    r.item = item;

    mRecords.append(r);

    ui->btnRemove->setEnabled(true);
    ui->listRegister->setCurrentRow(mRecords.count() - 1);
}


void DialogRegisterDevice::on_btnRemove_clicked()
{
    int current = ui->listRegister->currentRow();
    if( current < 0) {
        return;
    }
    QString txt = ui->listRegister->currentItem()->text();
    if( QMessageBox::Yes != QMessageBox::question(this, "Remove device?", QString("Are you sure you want to remove:%1").arg(txt)) ) {
        return;
    }

    delete mRecords[current].item;
    mRecords.removeAt(current);

    current--;
    if( current < 0 ) {
        current = 0;
    }

    if( ui->listRegister->count() ) {
        ui->listRegister->setCurrentRow(current);
    }else {
        ui->btnRemove->setEnabled(false);
    }
}


// TODO:
void DialogRegisterDevice::on_btnRegister_clicked()
{
    for( const Record& r: mRecords) {
        Animal* animal = nullptr;
        if( isNew() ) {
            if( mNames.contains(r.mName) ){
                qCritical() << QString("%1 already exists!").arg(r.mName);
                return;
            }

            animal = mHerd->newAnimal(r.mName,
                                      ui->radioMale->isChecked(),
                                      isCollar() ? r.mEui : "",
                                      isBolus() ? r.mEui : "" );
        }else {

            animal = mHerd->animal(r.mName);
            if( !animal) {
                qCritical() << QString("%1 already exists!").arg(name());
                return;
            }

            if( isCollar() ) {
                animal->putCollar(r.mEui.toUtf8().toBase64());
            }else
            if( isBolus() ) {
                animal->putBolus(r.mEui.toUtf8().toBase64());
            }else
            if( isRelay() ) {

            };
        }
    }

    mRecords.clear();
    ui->listRegister->clear();

    mIsChange = true;
}

void DialogRegisterDevice::on_btnCancel_clicked()
{
    if( mRecords.count()) {
        if( QMessageBox::Yes != QMessageBox::question(this, "Close registration?", QString("Are you sure you want to cancel all devices (%1) from registration?").arg(mRecords.count())) ) {
            return;
        }
    }

    close();
}

