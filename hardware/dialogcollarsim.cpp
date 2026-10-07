#include <QToolTip>
#include <QFileDialog>
#include <QPainter>
#include <QSerialPortInfo>

#include "hardware/collar/button.h"
#include "hardware/collar/defines.h"
#include "tools.h"
#include "collar/screen.h"
#include "collar/led.h"
#include "../animal.h"
#include "../mainwindow.h"
#include "dialogcollarsim.h"
#include "ui_dialogcollarsim.h"

#define DLGCOLLARSIM_MAX_COMMAND_STRINGS 10

uint32_t DialogCollarSim::mBaudrates[] =  {
    115200, 57600, 38400, 19200, 9600
};

void DialogCollarSim::createMirror(const QString& animalName, bool isMale, const QString& euiHex, const QString& akeyHex, const QString& nkeyHex )
{
    deleteMirror();
    if( !gMainWindow->isSimulation() && !mMirror ) {
        mMirror = new Collar(animalName, isMale,
                             QByteArray::fromHex(euiHex.toLatin1()),
                             QByteArray::fromHex(akeyHex.toLatin1()),
                             QByteArray::fromHex(nkeyHex.toLatin1()));
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


DialogCollarSim::DialogCollarSim(QSettings& env, QWidget *parent)
    : QDialog(parent), mEnv(env),
    ui(new Ui::DialogCollarSim)
{
    ui->setupUi(this);

    mImageDisconnected = QImage("://disconnected.png");

    mIconMale = QIcon("://male.svg");
    mIconFemale = QIcon("://female.svg");

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
        ui->comboAnimals->addItem(a->isMale() ? mIconMale: mIconFemale, a->name(), QVariant::fromValue(a));
    }

    mIsLoadingAnimals = false;

    ui->comboAnimals->setCurrentIndex(0);
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
            col = SCREEN_COL_MIRROR;

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
    if( !mAnimal) {
        return;
    }

    mAnimal->collar()->onMainBtn();
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

        // request keys
        //sendToSerial("eui", true);
        //sendToSerial("akey", true);
        //sendToSerial("nkey", true);

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
    ui->btnCopyAKey->setEnabled(checked);
    ui->btnFlash->setEnabled(checked);
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
    if( !gMainWindow->question("Are you sure you wanna flash the device?") ) {
        return;
    }

    if( ui->checkFlashEui->isChecked() ) {
        sendToSerial(QString("eui %1").arg(ui->editEui->text()));
    }

    sendToSerial(QString("akey %1").arg(ui->editAKey->text()));
    sendToSerial(QString("nkey %1").arg(ui->editNKey->text()));
    sendToSerial("flash");
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

        if( resp.length() < (cmd.length() + 2) ) {
            return false;
        }

        QString head = QString("%1:").arg(cmd);

        if( resp.left(head.length()) != head ) {
            return false;
        }

        resp = resp.right(resp.length() - head.length());

        return true;
    };

    auto fillOnResponce = [&](QString resp, const QString& cmd, QLineEdit* edit = nullptr) {

        if( !stripResponce(resp, cmd)) {
            return false;
        }

        if( mRequests.contains(cmd) ) {
            mRequests[cmd]--;
            if( !mRequests[cmd] ) {
                mRequests.remove(cmd);
            }
        }else{
            return true;
        }

        if( edit ) {
            edit->setText(resp);
        }

        return true;
    };


    if( mPort.isOpen() ) {
        if( mPort.bytesAvailable() ) {
            QByteArray out = mPort.readLine();
            QString resp = QString::fromLatin1(out);

            if( fillOnResponce(resp, "dbg") ) {
                if( !mMirror) {
                    return;
                }
                GeoPoint pos;
                int snr, rssi, bat, sat;

                QStringList params = resp.right(4).split("|");
                for(QString param : params) {
                    QStringList pair = param.split('=');
                    QStringList args = pair[1].split(",");
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
                    if( pair.first() == "btn") {
                        on_btnMain_pressed();
                    }
                }
                mMirror->inject(pos, sat, rssi, snr, bat);
                return;
            }else {
                addResponce(resp, RESPONCE_COLOR_RESPONCE);
            }

            if( stripResponce(resp, "info") ) {
                QStringList args = resp.split("|");
                if( !mMirror ) {
                    createMirror( args[0], args[1] == "m" ? true :  false, args[2], args[3], args[4]);
                }
                return;
            }

            if( fillOnResponce(resp, "eui", ui->editEui) ) {
                return;
            }

            if( fillOnResponce(resp, "eui", ui->editEui) ) {
                return;
            }

            if( fillOnResponce(resp, "akey", ui->editAKey) ) {
                return;
            }

            if( fillOnResponce(resp, "nkey", ui->editNKey) ) {
                return;
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

