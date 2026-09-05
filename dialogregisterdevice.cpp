#include "dialogregisterdevice.h"
#include "ui_dialogregisterdevice.h"

#include "hardware/loradev.h"
//#include "hardware/loradev_def.h"
//#include "hardware/gateway/gateway.h"
//#include "hardware/hardware/tools.h"
#include "simtools.h"
#include "herd.h"
#include <QToolTip>

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
    ui->editASKey->clear();
    ui->editNSKey->clear();
    ui->editName->clear();
    setStatus("", mStatusBackColor);

}

void DialogRegisterDevice::setStatus(const QString &txt, const QColor& col)
{
    ui->editStatus->setText(txt);

    QPalette palette = ui->editStatus->palette();
    palette.setColor(QPalette::Base,col);
    ui->editStatus->setPalette(palette);
}

DialogRegisterDevice::DialogRegisterDevice(Herd* herd, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogRegisterDevice)
{
    ui->setupUi(this);
    mHerd = herd;

    QPalette palette = ui->editStatus->palette();
    mStatusBackColor = palette.color(QPalette::Base);

    mNames = mHerd->names();

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

void DialogRegisterDevice::on_checkVirtual_toggled(bool checked)
{
    ui->btnDeviceRead->setEnabled(!checked);
    ui->groupBoxType->setEnabled(checked);

    ui->editEUI->setEnabled(checked);
    ui->editASKey->setEnabled(checked);
    ui->editNSKey->setEnabled(checked);

    ui->btnClearEUI->setEnabled(checked);
    ui->btnClearAKey->setEnabled(checked);
    ui->btnClearSKey->setEnabled(checked);

    ui->btnGenEUI->setEnabled(checked);
    ui->btnGenAKey->setEnabled(checked);
    ui->btnGenNKey->setEnabled(checked);

    //ui->btnCopyEUI->setEnabled(checked);
    //ui->btnCopyASKey->setEnabled(checked);
    //ui->btnCopyNSKey->setEnabled(checked);
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


void DialogRegisterDevice::on_btnGenAKey_clicked()
{
    QByteArray ba = SimTools::genAesKey();
    ui->editASKey->setText(ba.toUpper());
}


void DialogRegisterDevice::on_btnGenNKey_clicked()
{
    QByteArray ba = SimTools::genAesKey();
    ui->editNSKey->setText(ba.toUpper());

}


void DialogRegisterDevice::on_btnClearEUI_clicked()
{
    ui->editEUI->clear();
}


void DialogRegisterDevice::on_btnClearAKey_clicked()
{
    ui->editASKey->clear();
}


void DialogRegisterDevice::on_btnClearSKey_clicked()
{
    ui->editNSKey->clear();
}


void DialogRegisterDevice::on_btnCopyEUI_clicked()
{
    SimTools::clipboardCopy(ui->editEUI->text());
    QToolTip::showText( QCursor::pos(), "EUI copied!");
}


void DialogRegisterDevice::on_btnCopyASKey_clicked()
{
    SimTools::clipboardCopy(ui->editASKey->text());
    QToolTip::showText( QCursor::pos(), "ASKey copied!");
}


void DialogRegisterDevice::on_btnCopyNSKey_clicked()
{
    SimTools::clipboardCopy(ui->editNSKey->text());
    QToolTip::showText( QCursor::pos(), "NSKey copied!");
}


void DialogRegisterDevice::on_radioAnimalNew_toggled(bool checked)
{
    ui->editName->setVisible(checked);
    ui->radioFemale->setVisible(checked);
    ui->radioMale->setVisible(checked);

    ui->comboHerd->setVisible(!checked);
}


void DialogRegisterDevice::on_btnClear_clicked()
{
    clear();
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

// TODO:
void DialogRegisterDevice::on_btnRegister_clicked()
{
    Animal* animal = nullptr;
    if( isNew() ) {
        if( mNames.contains(name()) ){
            setStatus( QString("%1 already exists!").arg(name()), COL_ERR );
            return;
        }


        animal = mHerd->newAnimal(name(),
            ui->radioMale->isChecked(),
            isCollar() ? ui->editEUI->text() : "",
            isBolus() ? ui->editEUI->text() : "" );
    }else {

        animal = mHerd->animal(name());

        if( !animal ) {
            setStatus( QString("Animal %1 not found!").arg(name()), COL_ERR);
            return;
        }

        if( isCollar() ) {

        }else
        if( isBolus() ) {

        }else
        if( isRelay() ) {

        };

    }

    setStatus("Registering...", mStatusBackColor);
}

QString DialogRegisterDevice::name()
{
    return ui->editName->text();
}
