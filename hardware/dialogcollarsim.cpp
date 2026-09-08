#include "tools.h"
#include "../animal.h"


#include "dialogcollarsim.h"
#include "ui_dialogcollarsim.h"

DialogCollarSim::DialogCollarSim(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCollarSim)
{
    ui->setupUi(this);
}

void DialogCollarSim::init(QList<Animal *> animals)
{
    QIcon iconMale("://male.svg");
    QIcon iconFemale("://female.svg");

    ui->comboAnimals->clear();
    for(Animal* a:animals) {
        ui->comboAnimals->addItem(a->isMale() ? iconMale: iconFemale, a->name(), QVariant::fromValue(a));
    }

    ui->comboAnimals->setCurrentIndex(0);
}

DialogCollarSim::~DialogCollarSim()
{
    delete ui;
}


void DialogCollarSim::sendScreen(Animal *from)
{
    if( !mAnimal) {
        return;
    }
    if( mAnimal != from ) {
        return;
    }

    ScreenLib& lib = from->collar()->screen().lib();
    uint8_t* src = lib.mBuffer;
    int cx = ScreenSim::mH;
    int cy = ScreenSim::mW;

    for( auto y = 0; y < cy; y++) {
        for( auto x = 0; x < cx; x++) {
            mScreenImage.setPixelColor(x, y, src[y*cx + x] ? SCREEN_COL_LIGHT : SCREEN_COL_DARK);
        }
    }

    SimTools::setBtnImage(ui->btnScreen, mScreenImage);
}

void DialogCollarSim::on_comboAnimals_currentIndexChanged(int index)
{
    mAnimal = nullptr;
    if( index > 0 ) {
        return;
    }

    mAnimal = ui->comboAnimals->currentData().value<Animal*>();

    mScreenImage = QImage(ScreenSim::mW, ScreenSim::mH, QImage::Format_RGB888);
    mScreenImage.fill(SCREEN_COL_DARK);

}

