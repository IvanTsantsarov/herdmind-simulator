#include "dialogregisterdevice.h"
#include "ui_dialogregisterdevice.h"

#include "hardware/loradev.h"
//#include "hardware/loradev_def.h"
//#include "hardware/gateway/gateway.h"
//#include "hardware/hardware/tools.h"
#include "simtools.h"
#include <QToolTip>


DialogRegisterDevice::DialogRegisterDevice(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogRegisterDevice)
{
    ui->setupUi(this);
}

DialogRegisterDevice::~DialogRegisterDevice()
{
    delete ui;
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

    ui->btnCopyEUI->setEnabled(checked);
    ui->btnCopyASKey->setEnabled(checked);
    ui->btnCopyNSKey->setEnabled(checked);
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

