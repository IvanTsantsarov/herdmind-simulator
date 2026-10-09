#include <QToolTip>
#include <QFileDialog>
#include <QPainter>
#include <QSerialPortInfo>

#include "hardware/collar/button.h"
#include "hardware/collar/defines.h"
#include "hardware/collar/serialcmd.h"
#include "tools.h"
#include "devmanager.h"
#include "collar/screen.h"
#include "collar/led.h"
#include "../animal.h"
#include "../mainwindow.h"
#include "dialogcollarsim.h"
#include "ui_dialogcollarsim.h"

#define DLGCOLLARSIM_MAX_COMMAND_STRINGS 10

#define DLGCOLLARSIM_BACKCOL_OK QColor(255, 255, 255)
#define DLGCOLLARSIM_BACKCOL_ERROR QColor(235, 192, 188)


const QRegularExpression DialogCollarSim::mRegexEuiHex =
    QRegularExpression(R"(^[0-9a-fA-F]{16})");
const QRegularExpression DialogCollarSim::mRegexKeyHex =
    QRegularExpression(R"(^[0-9a-fA-F]{32})");


uint32_t DialogCollarSim::mBaudrates[] =  {
    115200, 57600, 38400, 19200, 9600
};

void DialogCollarSim::createMirror(const QString& animalName, bool isMale,
                                   const QString& euiHex,
                                   const QString& akeyHex,
                                   const QString& nkeyHex,
                                   const QString& addrHex )
{
    deleteMirror();
    if( !gMainWindow->isSimulation() ) {
        mMirror = new Collar(animalName, isMale,
                             QByteArray::fromHex( euiHex.toLatin1()),
                             QByteArray::fromHex( akeyHex.toLatin1()),
                             QByteArray::fromHex( nkeyHex.toLatin1()),
                            addrHex.toLatin1());
    }
}

void DialogCollarSim::deleteMirror()
{
    if( mMirror ) {
        delete mMirror;
        mMirror = nullptr;
    }
}

void DialogCollarSim::setLightsColor(const QColor &col)
{
    SimTools::setWidgetBackColor( ui->btnLed0, col);
    SimTools::setWidgetBackColor( ui->btnLed1, col);
    SimTools::setWidgetBackColor( ui->btnLed2, col);
    SimTools::setWidgetBackColor( ui->btnLed3, col);
    SimTools::setWidgetBackColor( ui->btnLed4, col);
    SimTools::setWidgetBackColor( ui->btnLed5, col);
    SimTools::setWidgetBackColor( ui->btnLed6, col);
    SimTools::setWidgetBackColor( ui->btnLed7, col);
}

void DialogCollarSim::closeEvent(QCloseEvent *e)
{
    (void)e;
    gMainWindow->onDlgCollarClose();
}


DialogCollarSim::DialogCollarSim(QSettings& env, DevManager* dm, QWidget *parent)
    : QDialog(parent), mDM(dm), mEnv(env),
    ui(new Ui::DialogCollarSim)
{
    ui->setupUi(this);

    mImageDisconnected = QImage("://disconnected.png");

    mIconMale = QIcon("://male.svg");
    mIconFemale = QIcon("://female.svg");
    mIconMaleCollar = QIcon("://male_new.svg");
    mIconFemaleCollar = QIcon("://female_new.svg");

    mIconSoundOn = QIcon("://icon-sound-on.svg");
    mIconSoundOff = QIcon("://icon-sound-off.svg");

    ui->btnBuzzer->setIcon(mIconSoundOff);

    ui->widgetScreen->configImage(SCREEN_CX, SCREEN_CY, SCREEN_COL_DARK);

    setLightsColor(Qt::black);
    SimTools::setWidgetBackColor( ui->btnLedMain, Qt::black);

    // In simulation mode SerialPort is not used
    ui->groupPorts->setVisible(!gMainWindow->isSimulation());
    if( !gMainWindow->isSimulation() ) {

        // Fill with all ports
        QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
        for( QSerialPortInfo& info:ports ) {
            ui->comboPorts->addItem(info.portName());
        }

        int baudratesCount = sizeof(mBaudrates)/sizeof(uint32_t);
        for( auto i = 0; i < baudratesCount; i ++) {
            ui->comboBaudrate->addItem(QString("%1").arg(mBaudrates[i]));
        }

        connect(&mPort, &QSerialPort::errorOccurred, this, &DialogCollarSim::on_serialPortError);
        connect(mDM, &DevManager::deviceActivated, this, &DialogCollarSim::on_deviceActivated );
    }


    // Hide for now LEDs
    ui->groupLEDs->setVisible(false);

    ui->widgetScreen->image() = mImageDisconnected;

}

void DialogCollarSim::loadAnimals(QList<Animal *> animals)
{
    mIsLoadingAnimals = true;

    mAnimal = nullptr;
    ui->comboAnimals->clear();
    for(Animal* a:animals) {
        ui->comboAnimals->addItem(a->isMale() ?
                                      (a->hasCollar() ? mIconMaleCollar : mIconMale) :
                                      (a->hasCollar() ? mIconFemaleCollar : mIconFemale),
                                    QString("%1%2").arg(a->name()).arg(a->hasCollar() && a->collar()->isActivated() ? "*" : ""),
                                    QVariant::fromValue(a));
    }

    mIsLoadingAnimals = false;

    if( mPrevAnimal.isEmpty() ) {
        ui->comboAnimals->setCurrentIndex(0);
    }else {
        int index = ui->comboAnimals->findText(mPrevAnimal);
        ui->comboAnimals->setCurrentIndex(index >= 0 ? index : 0);
    }
}

DialogCollarSim::~DialogCollarSim()
{
    deleteMirror();
    delete ui;
}

void DialogCollarSim::grabScreen()
{
    QImage& img = ui->widgetScreen->image();
    ScreenLib* lib = nullptr;
    QColor col;

    if( !gMainWindow->isSimulation() ) {
        if( mMirror ) {
            lib = &mMirror->screen()->lib();
            col = mMirror->screen()->isSleeping() ? SCREEN_COL_MIRROR_SLEEPING : SCREEN_COL_MIRROR;
        }
    }else {
        if( !mIsLoadingAnimals && mAnimal && mAnimal->hasCollar() ) {
            lib = &mAnimal->collar()->screen()->lib();
            col = mAnimal->collar()->screen()->isSleeping() ? SCREEN_COL_SLEEPING :  SCREEN_COL_LIGHT;
        }
    }

    if( !lib) {
        img = mImageDisconnected;
    }else {
        uint8_t* src = lib->mBuffer;

        for( auto y = 0; y < SCREEN_CY; y++) {
            for( auto x = 0; x < SCREEN_CX; x++) {
                img.setPixelColor(x, y, src[y*SCREEN_CX + x] ? col : SCREEN_COL_DARK);
            }
        }
    }

    ui->widgetScreen->update();
}



void DialogCollarSim::sendScreen(const Animal *from)
{
    if( !mAnimal) {
        return;
    }
    if( mAnimal != from ) {
        return;
    }

    grabScreen();
}

void DialogCollarSim::update()
{
    if( !gMainWindow->isSimulation()) {
        processSerialInput();
        grabScreen();
        return;
    }else
    {
        if( !mAnimal) {
            return;
        }

        // Simulation
        const bool isOn = mAnimal->collar()->led()->isOn();
        if( isOn != mIsLedOn ) {
            mIsLedOn = isOn;
            SimTools::setWidgetBackColor( ui->btnLedMain, isOn ? Qt::white : Qt::black);
        }

        QByteArray out = mAnimal->collar()->readFromSerial();
        if( out.length() ) {
            addResponce(QString::fromLatin1(out), RESPONCE_COLOR_RESPONCE);
        }
    }
}

void DialogCollarSim::on_comboAnimals_currentIndexChanged(int index)
{
    if( mIsLoadingAnimals) {
        return;
    }

    ui->btnStore->setEnabled(index >= 0);

    mAnimal = nullptr;
    if( index < 0 ) {
        return;
    }

    mAnimal = ui->comboAnimals->currentData().value<Animal*>();
    grabScreen();
}


void DialogCollarSim::on_btnGenArray_clicked()
{
    QString strSize = QString("%1x%2").arg(SCREEN_CX).arg(SCREEN_CY);
    QString title = QString("Choose image %1 max => C++ array"),arg(strSize);

    QString dirStr = mEnv.value("Collar/ArrayDir").toString();
    if( dirStr.isEmpty() )  {
        dirStr = "../../res";
    }
    QString imgPath = QFileDialog::getOpenFileName(this, title, dirStr, "*.png *.jpg *.jpeg *.bmp *.xpm" );

    if( imgPath.isEmpty()) {
        return;
    }

    mEnv.setValue("Collar/ArrayDir", dirStr);

    QImage img(imgPath);

    if( img.isNull() ) {
        gMainWindow->errorMsgBox("Image is invalid!");
        return;
    }

    QString imgSizeStr = QString("%1x%2").arg(img.width()).arg(img.height());

    if( img.width() > SCREEN_CX || img.height() > SCREEN_CY) {
        gMainWindow->errorMsgBox( QString("Image is bigger then %1 (%2)").arg(strSize).arg(imgSizeStr) );
        return;
    }

    QFileInfo fi(imgPath);

    QString varName = QString("uint8_t %1_%2[] = { ")
                          .arg(fi.baseName())
                          .arg(imgSizeStr);

    QString result = varName;

    for( int y = 0; y < img.height(); y ++) {
        for( int x = 0; x < img.width(); x ++) {

            if( img.hasAlphaChannel() ) {
                int alpha = qAlpha( img.pixel(x, y) );
                result.append(alpha ? "1," : "0,");
            }else {
                int col = qGray( img.pixel(x, y) );
                result.append(col ? "1," : "0,");
            }

        }
    }

    result.removeLast();
    result.append(" };");

    SimTools::clipboardCopy(result);

    gMainWindow->infoMsgBox( QString("C++ array definition of \"%1\" is in the clipboard! You can paste it in the cpp file and mention it as \"external\" in the Header file.").arg(varName) );

}


void ScreenWidget::paintEvent(QPaintEvent *event)
{
    (void) event;
    // Q_OBJECT // Optional macro check depending on your build setup
    QPainter painter(this);

    // CRITICAL: Disable smooth scaling to get sharp, nearest-neighbor pixel rendering
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setRenderHint(QPainter::Antialiasing, false);
    QImage scaledImg = mImage.scaled(rect().size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);

    // Draw the image filling the entire widget canvas rect
    painter.drawImage(0, 0, scaledImg);
}

void DialogCollarSim::on_btnMain_pressed()
{
    if( mMirror ) {
        mMirror->onMainBtn();
        return;
    }

    if( mAnimal) {
        mAnimal->collar()->onMainBtn();
        return;
    }
}

void DialogCollarSim::on_editSerialCmd_textChanged(const QString &arg1)
{
    ui->btnSend->setEnabled(arg1.length());
}

void DialogCollarSim::on_btnSend_clicked()
{
    sendToSerial();
}


void DialogCollarSim::on_editSerialCmd_returnPressed()
{
    sendToSerial();
}

void DialogCollarSim::sendToSerial(const QString& msg, bool isRequest)
{
    QString txt = msg.isEmpty() ? ui->editSerialCmd->text() : msg;
    if( txt.isEmpty() ) {
        return;
    }

    if( isRequest ) {
        if( mRequests.contains(msg) ) {
            mRequests[msg] ++;
        }else{
            mRequests[msg] = 1;
        }
    }

    QString txtTerm = QString( "%1\r").arg(txt);

    if( gMainWindow->isSimulation() ) {
        if( !mAnimal) {
            addResponce("Error:No animal selected.", RESPONCE_COLOR_ERROR);
            return;
        }

        mAnimal->collar()->sendToSerial(txtTerm.toLocal8Bit().data());
    }else {
        if( !mPort.isOpen() ) {
            addResponce("Error:Port not opened.", RESPONCE_COLOR_ERROR);
            return;
        }

        mPort.write(txtTerm.toLatin1());
        mPort.flush();
    }

    // delay(100);

    ui->editSerialCmd->clear();
    addResponce(">" + txt, RESPONCE_COLOR_COMMAND);
}

void DialogCollarSim::addResponce(const QString &txt, const QColor &c)
{
    ui->editResponce->setTextColor(c);
    ui->editResponce->append(txt);
}


void DialogCollarSim::on_checkConnect_toggled(bool checked)
{
    if( mIsOpeningPort ) {
        return;
    }

    if( checked ) {
        QString portName = ui->comboPorts->currentText();
        uint32_t baudrate = ui->comboBaudrate->currentText().toInt();

        QString log = QString("Opening serial port %1 @ %2 ..").arg(portName).arg(baudrate);
        qInfo() << log;
        addResponce(log, RESPONCE_COLOR_INFO);

        mIsOpeningPort = true;
        mPort.setPortName(portName);
        mPort.setBaudRate(baudrate);

        if( !mPort.open(QIODevice::ReadWrite) ) {
            addResponce( QString("Opening %1 port").arg(portName), RESPONCE_COLOR_ERROR );
            ui->checkConnect->setChecked(false);
            mIsOpeningPort = false;
            return;
        }

        log = QString("Port %1 open").arg(portName);
        addResponce( log, RESPONCE_COLOR_SUCESS );
        sendToSerial("info", true);

    }else {
        if( !mPort.isOpen() ) {
            mIsOpeningPort = false;
            return;
        }

        deleteMirror();

        QString log = QString("Closing serial port %1 @ %2 ..").arg(mPort.portName()).arg(mPort.baudRate());
        qInfo() << log;
        addResponce( log, RESPONCE_COLOR_INFO);

        mPort.close();
        mIsOpeningPort = false;
    }


    ui->editEui->setEnabled(checked);
    ui->btnGenEui->setEnabled(checked);
    ui->btnCopyEui->setEnabled(checked);
    ui->editAKey->setEnabled(checked);
    ui->btnGenAKey->setEnabled(checked);
    ui->btnCopyAKey->setEnabled(checked);
    ui->editNKey->setEnabled(checked);
    ui->btnGenNKey->setEnabled(checked);
    ui->btnCopyNKey->setEnabled(checked);
    ui->btnCopyAddr->setEnabled(checked);

    ui->comboPorts->setEnabled(!checked);
    ui->comboBaudrate->setEnabled(!checked);

    mIsOpeningPort = false;
}


void DialogCollarSim::on_btnGenAKey_clicked()
{
    QByteArray ba = SimTools::genHex(LORA_KEY_LEN);
    ui->editAKey->setText(ba.toUpper());
}


void DialogCollarSim::on_btnGenNKey_clicked()
{
    QByteArray ba = SimTools::genHex(LORA_KEY_LEN);
    ui->editNKey->setText(ba.toUpper());
}

void DialogCollarSim::on_btnGenEui_clicked()
{
    QByteArray ba = SimTools::genHex(LORA_EUI_LEN);
    ui->editNKey->setText(ba.toUpper());
}


void DialogCollarSim::on_btnFlash_clicked()
{
}

void DialogCollarSim::on_serialPortError(QSerialPort::SerialPortError err)
{
    if( QSerialPort::NoError == err ) {
        // Why the f*ck you sending no error?!
        return;
    }

    QString log = QString("Serial port error:(%1) %2").arg(err).arg(mPort.errorString());
    addResponce(log, RESPONCE_COLOR_ERROR);
    qWarning() << log;
}


void DialogCollarSim::on_btnClear_clicked()
{
    ui->editResponce->clear();
}



void DialogCollarSim::processSerialInput()
{
    auto stripResponce = [&](QString& resp, const QString& cmd) {
        resp = resp.trimmed();

        if( resp.length() < (cmd.length() + 3) ) {
            return false;
        }

        QString head = QString("%1%2:").arg(SERIAL_CMD_BEGIN).arg(cmd);

        if( resp.left(head.length()) != head ) {
            return false;
        }

        resp = resp.right(resp.length() - head.length());

        return true;
    };


    if( mPort.isOpen() ) {
        if( mPort.bytesAvailable() && mPort.canReadLine()) {

            QByteArray out = mPort.readLine();
            QString resp = QString::fromLatin1(out);

            if( SERIAL_CMD_BEGIN != resp[0] ) {
                return;
            }

            if( stripResponce(resp, "dbg") ) {
                if( !mMirror) {
                    return;
                }
                GeoPoint pos;
                int snr, rssi, bat, sat;

                QStringList params = resp.split(SERIAL_CMD_PARAMS_DM);
                for(QString param : params) {
                    QStringList pair = param.split(SERIAL_CMD_EQUAL);
                    QStringList args = pair[1].split(SERIAL_CMD_COMMA);
                    if( pair.first() == "gps") {
                        sat = args[0].toInt();
                        pos.mLat = args[1].toFloat();
                        pos.mLon = args[2].toFloat();
                    }else
                    if( pair.first() == "rssi") {
                        rssi = args[0].toInt();
                    }else
                    if( pair.first() == "snr") {
                        snr = args[0].toInt();
                    }else
                    if( pair.first() == "bat") {
                        bat = args[1].remove("%").toInt();
                    }else
                    if( pair.first() == "btn" && pair[1] == "y") {
                        on_btnMain_pressed();
                    }
                }
                mMirror->inject(pos, sat, rssi, snr, bat);
                return;
            }else
            if( stripResponce(resp, "info") ) {
                QStringList args = resp.split("|");
                ui->editAnimalName->setText(args[0]);
                ui->btnSex->setIcon(args[1] == "m" ? mIconMale : mIconFemale );
                ui->editEui->setText(args[2].toUpper());
                ui->editAKey->setText(args[3].toUpper());
                ui->editNKey->setText(args[4].toUpper());
                ui->editAddr->setText(args[5].toUpper());
                ui->btnReload->setEnabled(false);
                ui->btnStore->setEnabled( ui->comboAnimals->currentIndex() >= 0 );
                if( !mMirror ) {
                    createMirror( args[0], args[1] == "m" ? true :  false, args[2], args[3], args[4], args[5]);
                }
                return;
            }else
            {
                addResponce(resp, RESPONCE_COLOR_RESPONCE);
            }
        }
    }
}


void DialogCollarSim::on_btnReset_clicked()
{
    sendToSerial("reset");
}

void DialogCollarSim::on_btnCopyEui_clicked()
{
    SimTools::clipboardCopy(ui->editEui->text());
    QToolTip::showText( QCursor::pos(), "EUI copied!");
}


void DialogCollarSim::on_btnCopyAKey_clicked()
{
    SimTools::clipboardCopy(ui->editAKey->text());
    QToolTip::showText( QCursor::pos(), "AppKey copied!");
}


void DialogCollarSim::on_btnCopyNKey_clicked()
{
    SimTools::clipboardCopy(ui->editNKey->text());
    QToolTip::showText( QCursor::pos(), "NwkKey copied!");
}

void DialogCollarSim::on_btnCopyAddr_clicked()
{
    SimTools::clipboardCopy(ui->editAddr->text());
    QToolTip::showText( QCursor::pos(), "Address copied!");
}


void DialogCollarSim::setBackgroundError(QLineEdit *edit, bool isError)
{
    QPalette p = edit->palette();
    p.setColor(QPalette::Base, isError ? DLGCOLLARSIM_BACKCOL_ERROR : DLGCOLLARSIM_BACKCOL_OK); // BG
    edit->setPalette(p);
}

bool DialogCollarSim::isFlashDataValid()
{
    return  mRegexEuiHex.match(ui->editEui->text()).hasMatch() &&
            mRegexKeyHex.match(ui->editAKey->text()).hasMatch() &&
            mRegexKeyHex.match(ui->editNKey->text()).hasMatch();
}

void DialogCollarSim::on_editEui_textChanged(const QString &newEui)
{
    ui->btnReload->setEnabled(true);
    DialogCollarSim::setBackgroundError(ui->editEui, !mRegexEuiHex.match(newEui).hasMatch());
}


void DialogCollarSim::on_editAKey_textChanged(const QString &newKey)
{
    ui->btnReload->setEnabled(true);
    DialogCollarSim::setBackgroundError(ui->editAKey, !mRegexKeyHex.match(newKey).hasMatch());
}


void DialogCollarSim::on_editNKey_textChanged(const QString &newKey)
{
    ui->btnReload->setEnabled(true);
    DialogCollarSim::setBackgroundError(ui->editNKey, !mRegexKeyHex.match(newKey).hasMatch());
}


void DialogCollarSim::on_btnReload_clicked()
{
    sendToSerial("info", true);
}


void DialogCollarSim::on_btnStore_clicked()
{
    if( !mMirror ) {
        gMainWindow->errorMsgBox("Not connected to collar with a serial.");
        return;
    }

    if( !mAnimal) {
        QString txt( QString("No animal selected.%1").arg( ui->comboAnimals->count() ? "Pick one from the combo" : "Load from the main window.") );
        gMainWindow->errorMsgBox(txt);
        return;
    }

    if( !isFlashDataValid() ) {
        gMainWindow->errorMsgBox("Flash data not valid. Correct red fields");
        return;
    }

    Animal * removeCollarAnimal = nullptr;

    // Check for other animals collars
    for( int i = 0; i < ui->comboAnimals->count(); i ++) {
        Animal* a = ui->comboAnimals->itemData(i).value<Animal*>();
        if( !a->hasCollar() ) {
            continue;
        }

        if( a->collar()->eui() == mMirror->eui()) {
            if( !gMainWindow->question( QString("Animal %1 has the same collar. We need to remove it, ok?").arg(a->name())) ) {
                return;
            }else {
                removeCollarAnimal = a;
                break;
            }

        }
    }

    if( !gMainWindow->question( QString("Are you sure you wanna %1 animal %2?")
            .arg(mAnimal->hasCollar() ? "to replace current collar of" : "apply this collar to the")
            .arg(mAnimal->name())) ) {
        return;
    }

    if( removeCollarAnimal ) {
        if( !removeCollarAnimal->removeCollar() ) {
            gMainWindow->errorMsgBox( QString("Animal %1 collar removal error").arg(removeCollarAnimal->name()) );
            return;
        }
    }

    // Update mirror collar
    mMirror->setAnimalName(mAnimal->name());
    mMirror->setIsMale(mAnimal->isMale());
    mMirror->setKeysHex( ui->editEui->text(), ui->editAddr->text(), ui->editAKey->text(), ui->editNKey->text());
    mAnimal->putCollar(mMirror);
    mPrevAnimal = mAnimal->name();

    // This will gonna call void DialogCollarSim::loadAnimals(QList<Animal *> animals)
    // and fill the list again
    if( !gMainWindow->storeHerd() ) {
        gMainWindow->errorMsgBox("Error storing herd!");
    };
}

void DialogCollarSim::on_deviceActivated(LoraDevSim *dev)
{
    if( mMirror && (dev->eui() == mMirror->eui())) {

        QString addr = dev->addr().toHex();

        ui->editAddr->setText( addr );
        ui->editAnimalName->setText( mMirror->animalName().toQString() );
        mMirror->setAddrHex(addr.toLatin1());

        if( ui->checkFlashEui->isChecked() ) {
            sendToSerial(QString("eui %1").arg(ui->editEui->text()));
        }

        sendToSerial(QString("name %1").arg(mMirror->animalName().toQString()) );
        sendToSerial(QString("sex %1").arg(mMirror->isMale() ? "m" : "f"));
        sendToSerial(QString("akey %1").arg(ui->editAKey->text()));
        sendToSerial(QString("nkey %1").arg(ui->editNKey->text()));
        sendToSerial(QString("addr %1").arg(addr));
        sendToSerial("flash");

        QString str = QString("Collar of %1 (%2) activated! Address: %3")
                          .arg(mMirror->animalName().toQString())
                          .arg(mMirror->euiHex())
                          .arg(addr);

        QToolTip::showText( QCursor::pos(), str, this);
    }
}



