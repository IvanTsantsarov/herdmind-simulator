#include "dialogcollarsim.h"
#include "ui_dialogcollarsim.h"

DialogCollarSim::DialogCollarSim(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCollarSim)
{
    ui->setupUi(this);
}

DialogCollarSim::~DialogCollarSim()
{
    delete ui;
}
