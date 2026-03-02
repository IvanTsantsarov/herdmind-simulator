#include <QTableWidgetItem>
#include "apirest.h"
#include "hardware/loradev.h"
#include "mainwindow.h"
#include "devmanager.h"
#include "dialogdevicemsg.h"
#include "ui_dialogdevicemsg.h"

#define DLG_MSG_TABLE_COLS 4
#define DLG_MSG_TABLE_SOUND_COL 0
#define DLG_MSG_TABLE_LIGHT_COL 1
#define DLG_MSG_TABLE_SHOCK_COL 2
#define DLG_MSG_TABLE_NAME_COL 3


void DialogDeviceMsg::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    gMainWindow->onDeviceMsgClose();
}

QString DialogDeviceMsg::currentEUI()
{
    QTableWidgetItem* item = ui->tableDevices->item( ui->tableDevices->currentRow(), DLG_MSG_TABLE_NAME_COL );
    return item->data(Qt::UserRole).toString();
}

LoraDev *DialogDeviceMsg::deviceByRow(int row)
{
    QTableWidgetItem* item = ui->tableDevices->item(row, DLG_MSG_TABLE_NAME_COL);
    return mDevManager->device( item->data(Qt::UserRole).toString() );

}

QIcon DialogDeviceMsg::loadIcon(const QString &iconName)
{
    return QIcon(QPixmap(QString("://%1.svg").arg(iconName)));
}


DialogDeviceMsg::DialogDeviceMsg(DevManager *devManager, QWidget *parent)
    : QDialog(parent), mDevManager(devManager)
    , ui(new Ui::DialogDeviceMsg)
{
    ui->setupUi(this);

    mIconLightOn = loadIcon("icon-light-on");
    mIconLightOff = loadIcon("icon-light-off");;
    mIconSoundOn = loadIcon("icon-sound-on");
    mIconSoundOff = loadIcon("icon-sound-off");;
    mIconShockOn = loadIcon("icon-shock-on");
    mIconShockOff = loadIcon("icon-shock-off");

    QStringList ls = {"S", "L", "E", "Name" };
    ui->tableDevices->setHorizontalHeaderLabels(ls);
}

DialogDeviceMsg::~DialogDeviceMsg()
{
    delete ui;
}

void DialogDeviceMsg::updateDevices()
{
    // performs deep copy
    QList<QString> devices = mDevManager->devices();

    // ui->tableDevices->clear();

    ui->tableDevices->setColumnCount(DLG_MSG_TABLE_COLS);
    ui->tableDevices->setRowCount(devices.count());

    int row = 0;
    for(const QString& eui : devices) {
        ui->tableDevices->setItem(row, DLG_MSG_TABLE_SOUND_COL, new QTableWidgetItem(mIconSoundOff, ""));
        ui->tableDevices->setItem(row, DLG_MSG_TABLE_LIGHT_COL, new QTableWidgetItem(mIconLightOff, ""));
        ui->tableDevices->setItem(row, DLG_MSG_TABLE_SHOCK_COL, new QTableWidgetItem(mIconShockOff, ""));

        LoraDev* dev = mDevManager->device(eui);
        QTableWidgetItem* itemName = new QTableWidgetItem(dev->name());
        itemName->setToolTip(QString("0x%1 0x%2").arg(dev->addr().toHex()).arg(dev->eui().toHex()));
        itemName->setData(Qt::UserRole, eui);
        ui->tableDevices->setItem(row, DLG_MSG_TABLE_NAME_COL, itemName);
        row ++;
    }

    ui->tableDevices->resizeColumnsToContents();

    QHeaderView * header = ui->tableDevices->horizontalHeader();

    header->setSectionResizeMode(DLG_MSG_TABLE_SOUND_COL, QHeaderView::Fixed);
    header->setSectionResizeMode(DLG_MSG_TABLE_LIGHT_COL, QHeaderView::Fixed);
    header->setSectionResizeMode(DLG_MSG_TABLE_SHOCK_COL, QHeaderView::Fixed);
    header->setSectionResizeMode(DLG_MSG_TABLE_NAME_COL, QHeaderView::Stretch);

    ui->tableDevices->setCurrentItem(ui->tableDevices->item(0, DLG_MSG_TABLE_NAME_COL));
}

QString DialogDeviceMsg::getSelectedDeviceEUI()
{
    int row = ui->tableDevices->currentRow();
    QTableWidgetItem* item = ui->tableDevices->item(row, DLG_MSG_TABLE_NAME_COL);
    return item->data(Qt::UserRole).toString();
}



void DialogDeviceMsg::onResponse(const QString &devEUI, const QJsonObject &jobjResponse)
{
    QJsonDocument Doc(jobjResponse);
    ui->editResponse->appendPlainText(QString("%1|%2").arg(devEUI).arg(Doc.toJson()));
}

void DialogDeviceMsg::on_btnSendLight_clicked()
{
    QString eui = currentEUI();
    QString msg = QString("Lights ON");
    mDevManager->sendMessage( currentEUI(), msg.toUtf8() );
}


void DialogDeviceMsg::on_btnSendSound_clicked()
{
    QString eui = currentEUI();
    QString msg = QString("Sound ON");
    mDevManager->sendMessage( currentEUI(), msg.toUtf8() );
}


void DialogDeviceMsg::on_btnSendShock_clicked()
{
    QString eui = currentEUI();
    QString msg = QString("Shock ON");
    mDevManager->sendMessage( currentEUI(), msg.toUtf8() );
}


void DialogDeviceMsg::on_tableDevices_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    if( !current ) {
        return;
    }

    LoraDev* dev = deviceByRow( current->row() );
    if( !dev ) {
        qCritical() << "DialogDeviceMsg::on_tableDevices_currentItemChanged: Device unavailable";
        return;
    }

    ui->groupCollar->setEnabled( dev->isCollar() );
}

