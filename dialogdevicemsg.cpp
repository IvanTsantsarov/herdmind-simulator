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

    ui->tableDevices->clear();
}


void DialogDeviceMsg::updateDevices()
{
    clearConnections();

    // performs deep copy
    QList<QString> devices = mDevManager->devices();

    // ui->tableDevices->clear();

    ui->tableDevices->setColumnCount(DLG_MSG_TABLE_COLS);
    ui->tableDevices->setRowCount(devices.count());

    auto createItemButton = [=](int row, int column,
                                const QIcon& icon,
                                const QString& eui,
                                Protocol::Collar::Event event) {
        // ui->tableDevices->setItem(row, column, new QTableWidgetItem(mIconSoundOff, ""));

        QPushButton* btn = new QPushButton(this);
        btn->setIcon(icon);
        ui->tableDevices->setCellWidget(row, column, btn);

        btn->setProperty("eui", eui);
        btn->setProperty("msg", (int)event);

        connect(btn, &QPushButton::clicked, this, &DialogDeviceMsg::onDeviceBtnClicked );
    };


    int row = 0;
    for(const QString& eui : devices) {
        LoraDev* dev = mDevManager->device(eui);

        if( dev->isCollar() ) {
            QString eui = dev->eui().toHex();
            createItemButton( row, DLG_MSG_TABLE_LIGHT_COL, mIconLightOff, eui, Protocol::Collar::Event::Light );
            createItemButton( row, DLG_MSG_TABLE_SOUND_COL, mIconSoundOff, eui, Protocol::Collar::Event::Sound );
            createItemButton( row, DLG_MSG_TABLE_SHOCK_COL, mIconShockOff, eui, Protocol::Collar::Event::Shock );
        }

        DevCon con;
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


bool DialogDeviceMsg::changeDeviceMsgIcon(int row, Protocol::Collar::Event event, bool isOn)
{
    int col;
    QIcon icon;

    switch (event) {

    case Protocol::Collar::Event::Light:
        icon = isOn ? mIconLightOn : mIconLightOff;
        col = 0;
        break;

    case Protocol::Collar::Event::Sound:
        icon = isOn ? mIconSoundOn : mIconSoundOff;
        col = 1;
        break;

    case Protocol::Collar::Event::Shock:
        icon = isOn ? mIconShockOn : mIconShockOff;
        col = 2;
        break;

    default:
        return false;
    }


    QPushButton* btn = qobject_cast<QPushButton*>(ui->tableDevices->cellWidget(row, col));
    btn->setIcon(icon);
    return true;
}


void DialogDeviceMsg::onDeviceMessageAfter(const QByteArray &addr, const QByteArray &msg)
{
    DevCon con = mDevsMap[addr];
    changeDeviceMsgIcon(con.mRow, (Protocol::Collar::Event) msg[0], false);
}


void DialogDeviceMsg::onDeviceMessage(const QByteArray &addr, const QByteArray &msg)
{
    DevCon con = mDevsMap[addr];
    changeDeviceMsgIcon(con.mRow, (Protocol::Collar::Event)msg[0], true);
    QTimer::singleShot(MSG_AFTER_INTERVAL, [=]() {
        onDeviceMessageAfter(addr, msg);
    });
}

void DialogDeviceMsg::onDeviceBtnClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn)
        return;

    QString eui = btn->property("eui").toString();
    uint8_t event = btn->property("msg").toUInt();
    QByteArray msg;
    msg.append(event);

    mDevManager->sendMessage( eui, msg );
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

