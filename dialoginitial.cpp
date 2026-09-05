#include "dialoginitial.h"
#include "ui_dialoginitial.h"

DialogInitial::DialogInitial(QSettings& env, QWidget *parent) :
    QDialog(parent), mEnv(env)
    , ui(new Ui::DialogInitial)
{

    ui->setupUi(this);

    bool isRemoteSession = mEnv.value("Main/Remote").toBool();
    ui->radioRemote->setChecked(isRemoteSession);

    bool isSimulation = mEnv.value("Main/Simulation").toBool();
    ui->checkSimulation->setChecked(isSimulation);
}

DialogInitial::~DialogInitial()
{
    delete ui;
}

bool DialogInitial::isLocal()
{
    return ui->radioLocal->isChecked();
}

bool DialogInitial::isRemote()
{
    return ui->radioRemote->isChecked();
}

bool DialogInitial::isSimulation()
{
    return ui->checkSimulation->isChecked();
}

void DialogInitial::on_buttonBox_accepted()
{
    mIsOk = true;
    mEnv.setValue("Main/Remote", isRemote());
    mEnv.setValue("Main/Simulation", isSimulation());
}

