#include <QToolTip>
#include <QFileDialog>
#include <QPainter>

#include "hardware/collar/button.h"
#include "tools.h"
#include "collar/screen.h"
#include "../animal.h"
#include "../mainwindow.h"
#include "dialogcollarsim.h"
#include "ui_dialogcollarsim.h"

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

    mIconMale = QIcon("://male.svg");
    mIconFemale = QIcon("://female.svg");

    mIconSoundOn = QIcon("://icon-sound-on.svg");
    mIconSoundOff = QIcon("://icon-sound-off.svg");

    ui->btnBuzzer->setIcon(mIconSoundOff);

    ui->widgetScreen->configImage(SCREEN_CX, SCREEN_CY, SCREEN_COL_DARK);

    setLightsColor(QColor(0, 0, 0));
}

void DialogCollarSim::init(QList<Animal *> animals)
{

    ui->comboAnimals->clear();
    for(Animal* a:animals) {
        ui->comboAnimals->addItem(a->isMale() ? mIconMale: mIconFemale, a->name(), QVariant::fromValue(a));
    }

    ui->comboAnimals->setCurrentIndex(0);
}

DialogCollarSim::~DialogCollarSim()
{
    delete ui;
}

void DialogCollarSim::grabScreen()
{
    if( !mAnimal) {
        return;
    }

    ScreenLib& lib = mAnimal->collar()->screen()->lib();
    uint8_t* src = lib.mBuffer;

    QImage& img = ui->widgetScreen->image();

    QColor col = mAnimal->collar()->screen()->isSleeping() ? SCREEN_COL_SLEEPING :  SCREEN_COL_LIGHT;

    for( auto y = 0; y < SCREEN_CY; y++) {
        for( auto x = 0; x < SCREEN_CX; x++) {
            img.setPixelColor(x, y, src[y*SCREEN_CX + x] ? col : SCREEN_COL_DARK);
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

void DialogCollarSim::on_comboAnimals_currentIndexChanged(int index)
{
    mAnimal = nullptr;
    if( index > 0 ) {
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

