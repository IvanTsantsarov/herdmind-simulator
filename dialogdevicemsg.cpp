#include <QTableWidgetItem>
#include "apirest.h"
#include "hardware/loradev.h"
#include "mainwindow.h"
#include "devmanager.h"
#include "dialogdevicemsg.h"
#include "ui_dialogdevicemsg.h"

#define DLG_MSG_TABLE_COLS 4
#define DLG_MSG_TABLE_LIGHT_COL 0
#define DLG_MSG_TABLE_SOUND_COL 1
#define DLG_MSG_TABLE_SHOCK_COL 2
#define DLG_MSG_TABLE_NAME_COL 3

#define MSG_LIGHT_ON "Light ON"
#define MSG_SOUND_ON "Sound ON"
#define MSG_SHOCK_ON "Shock ON"

#define MSG_AFTER_INTERVAL 2000

void DialogDeviceMsg::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    gMainWindow->onDeviceMsgClose();
}

QString DialogDeviceMsg::currentEUI()
{
    QTableWidgetItem* item = ui->tableDevices->item( ui->tableDevices->currentRow(), DLG_MSG_TABLE_NAME_COL );
    if( !item) {
        return "";
    }
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

void DialogDeviceMsg::clearConnections()
{
    for(auto key: mDevsMap.keys()) {
        disconnect(mDevsMap[key].mConn);
    }
}


void DialogDeviceMsg::updateDevices()
{
    clearConnections();

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

        DevCon con;
        LoraDev* dev = mDevManager->device(eui);
        QTableWidgetItem* itemName = new QTableWidgetItem(dev->name());
        itemName->setToolTip(QString("0x%1 0x%2").arg(dev->addr().toHex()).arg(dev->eui().toHex()));
        itemName->setData(Qt::UserRole, eui);
        ui->tableDevices->setItem(row, DLG_MSG_TABLE_NAME_COL, itemName);

        con.mRow = row++;
        con.mConn = connect( dev, &LoraDev::messageReceived, this, &DialogDeviceMsg::onDeviceMessage );
        mDevsMap[dev->addr()] = con;

    }

    ui->tableDevices->resizeColumnsToContents();

    QHeaderView * header = ui->tableDevices->horizontalHeader();

    header->setSectionResizeMode(DLG_MSG_TABLE_LIGHT_COL, QHeaderView::Fixed);
    header->setSectionResizeMode(DLG_MSG_TABLE_SOUND_COL, QHeaderView::Fixed);
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


bool DialogDeviceMsg::changeDeviceMsgIcon(int row, const QByteArray &msg, bool isOn)
{
    int col;
    QIcon icon;

    if( msg == MSG_LIGHT_ON ) {
        icon = isOn ? mIconLightOn : mIconLightOff;
        col = 0;
    }else
    if( msg == MSG_SOUND_ON ) {
        icon = isOn ? mIconSoundOn : mIconSoundOff;
        col = 1;
    }else
    if( msg == MSG_SHOCK_ON )
    {
        icon = isOn ? mIconShockOn : mIconShockOff;
        col = 2;
    }else {
        return false;
    }

    QTableWidgetItem* item = ui->tableDevices->item(row, col);
    item->setIcon(icon);
    return true;
}


void DialogDeviceMsg::onDeviceMessageAfter(const QByteArray &addr, const QByteArray &msg)
{
    DevCon con = mDevsMap[addr];
    changeDeviceMsgIcon(con.mRow, msg, false);
}


void DialogDeviceMsg::onDeviceMessage(const QByteArray &addr, const QByteArray &msg)
{
    DevCon con = mDevsMap[addr];
    changeDeviceMsgIcon(con.mRow, msg, true);
    QTimer::singleShot(MSG_AFTER_INTERVAL, [=]() {
        onDeviceMessageAfter(addr, msg);
    });
}

void DialogDeviceMsg::on_btnSendLight_clicked()
{
    QString eui = currentEUI();
    QString msg = QString(MSG_LIGHT_ON);
    mDevManager->sendMessage( currentEUI(), msg.toUtf8() );
}


void DialogDeviceMsg::on_btnSendSound_clicked()
{
    QString eui = currentEUI();
    QString msg = QString(MSG_SOUND_ON);
    mDevManager->sendMessage( currentEUI(), msg.toUtf8() );
}


void DialogDeviceMsg::on_btnSendShock_clicked()
{
    QString eui = currentEUI();
    QString msg = QString(MSG_SHOCK_ON);
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


void DialogDeviceMsg::on_btnSend_clicked()
{
    if( mDevManager->isReady() ) {
        mDevManager->sendMessage( currentEUI(), ui->editSend->text().toUtf8() );
        ui->editSend->clear();
    }else {
        ui->editResponse->appendPlainText("Devices are not ready!");
    }
}

