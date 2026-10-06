#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QScreen>
#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QFile>
#include "dialogconsole.h"
#include "dialogdevicemsg.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scene.h"
#include "sceneview.h"
#include "herd.h"
#include "defines.h"
#include "animal.h"
#include "network.h"
#include "devmanager.h"
#include "simtools.h"
#include "simtimer.h"
#include "hardware/defines.h"
#include "dialogregisteranimal.h"
#include "dialogsettings.h"
#include "hardware/dialogcollarsim.h"

#define TABLE_COLS_COUNT 3
#define REMINDER_DELAY 3000

MainWindow* gMainWindow = nullptr;

MainWindow::MainWindow(bool isSim, QSettings &env, QSettings &settings, QWidget *parent)
    : QMainWindow(parent), mIsSimulation(isSim), mEnv(env), mSettings(settings)
    , ui(new Ui::MainWindow)
{
    mConsole = new DialogConsole(settings, this);
    gMainWindow = this;

    ui->setupUi(this);

    setWindowTitle(mIsSimulation ? "Herdmind Simulation" : "Herdmind real");

    ui->groupSimulation->setVisible(mIsSimulation);
    ui->btnAdd->setVisible(!mIsSimulation);

    mDlgSettings = new DialogSettings(settings, this);
    if( !mDlgSettings->checkValues()) {
        mDlgSettings->exec();
    }

    // create scene
    mScene = new Scene(this);
    mScene->setSceneRect(-INITIAL_MEDDOW_SIZE, -INITIAL_MEDDOW_SIZE, 2*INITIAL_MEDDOW_SIZE, 2*INITIAL_MEDDOW_SIZE);

    mSceneView = new SceneView(mScene, this);
    ui->mainVerticalLayout->addWidget(mSceneView);
    mSceneView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Table initialization
    ui->table->setAlternatingRowColors(true);
    ui->table->setStyleSheet("\
        QTableView { \
            selection-background-color: #0078d7;\
            selection-color: white;\
            alternate-background-color: #f7f7f7;\
        }\
        QTableView::item:selected:active {\
            background-color: #0078d7;\
            color: white;\
        }\
        QTableView::item:selected:!active {\
            background-color: #9cc9ff;\
            color: black;\
        }");

    initGenerationUI();

    QObject::connect(&mUpdateTimer, &QTimer::timeout, this, &MainWindow::onUpdate );
    mUpdateTimer.start(HERD_UPDATE_INTERVAL);

    // Connect the cell click signal to your custom slot
    connect(ui->table, &QTableWidget::cellClicked,
            this, &MainWindow::onRowClicked);


    ui->widgetSim->setVisible(false);
    ui->widgetGrazing->setVisible(false);
    ui->widgetPastureGen->setVisible(false);
    showMaximized();

    // ui->scrollAreaParams->setWidgetResizable(false); // chatGPT was wrong about this

    QString animalListFile = isSimulation() ? ANIMALS_LIST_FILE_SIM : ANIMALS_LIST_FILE;

    // try to load saved properties
    if( gSimTools->fileExists(animalListFile)) {
        qInfo() << "Animal list file available:" << animalListFile;
        ui->btnLoad->setEnabled(true);
    }else {
        qInfo() << "File not exists:" << animalListFile;
        ui->btnLoad->setEnabled(false);
    }

    mDevManager = new DevManager(settings);

    mDevMsg = new DialogDeviceMsg(mDevManager, this);

    mDlgCollar = new DialogCollarSim(env, this);
    gTools.setup(mDlgCollar);

    QRect screenrect = qApp->primaryScreen()->geometry();
    mConsole->move(screenrect.left(), screenrect.bottom()/2);
    mDevMsg->move(screenrect.right()/2, screenrect.bottom()/2);

    // restore environment
    bool is = mEnv.value("UI/Console").toBool();
    mConsole->setVisible( is );
    ui->actionConsole->setChecked(is);

    is = mEnv.value("UI/DevMsg").toBool();
    mDevMsg->setVisible( is );
    ui->actionDeviceMsg->setChecked(is);

    is = mEnv.value("UI/CollarSim").toBool();
    mDlgCollar->setVisible( is );
    ui->actionDlgCollar->setChecked(is);


    is = mEnv.value("UI/GroupFold").toBool();
    ui->btnShowInfo->setChecked(is);
    ui->groupFold->setVisible(is);

    is = mEnv.value("UI/isGrowing").toBool();
    ui->checkGrowingMeadow->setChecked(is);

    is = mEnv.value("UI/DebugInfo").toBool();
    mConsole->setDebugInfo(is);


#if PRINT_DEBUG_INFO == false
    ui->checkRecursiveCollision->setVisible(false);
#endif

    // is isLoadLast value is set to 1 in settings.ini then load last herd
    if( settings.value("GUI/isLoadLast").toBool() ) {
        create(true);
    }

    ui->radioAnimalsCount->setChecked(!isSim);
    ui->radioAnimalsPercentage->setChecked(isSim);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    mEnv.setValue("UI/Console", mConsole->isVisible() );
    mEnv.setValue("UI/DevMsg", mDevMsg->isVisible() );
    mEnv.setValue("UI/CollarSim", mDlgCollar->isVisible() );
    mEnv.setValue("UI/GroupFold", ui->btnShowInfo->isChecked());
    mEnv.setValue("UI/isGrowing", ui->checkGrowingMeadow->isChecked());
    mEnv.setValue("UI/DebugInfo", mConsole->isDebugInfo());
    mScene->saveFence();
}



bool MainWindow::create(bool isLoad, const QString& dir)
{
    mIsCreated = false;

    mSceneView->setMeadow(nullptr);

    if( mHerd) {
        delete mHerd;
        mHerd = nullptr;
    }

    if( mMeadow) {
        delete mMeadow;
        mMeadow = nullptr;
    }

    if( mNetwork) {
        delete mNetwork;
        mNetwork = nullptr;
    }

    ui->checkShepard->setChecked(false);

    SimTools::HarmonicsGenerator::Params pastureParams;

    pastureParams.radius = ui->spinPastureGenRadius->value();
    pastureParams.count = ui->spinPastureGenCount->value();
    pastureParams.ampMin = ui->spinPastureGenAmpMin->value();
    pastureParams.ampMax = ui->spinPastureGenAmpMax->value();
    pastureParams.wavelenMin = ui->spinPastureGenWaveMin->value();
    pastureParams.wavelenMax = ui->spinPastureGenWaveMax->value();

    // Generate meadow
    mMeadow = new Meadow(QPoint(0, 0),
                         QGeoCoordinate(ui->spinCenterLat->value(), ui->spinCenterLong->value()),
                         QSize( ui->spinMeadowDimX->value(), ui->spinMeadowDimY->value()),
                         ui->spinLawnRadius->value(),
                         ui->spinMeadowCapacity->value(),
                         ui->spinMeadowGrowingSpeed->value(),
                         ui->spinAnimalsPerLawn->value(),
                         pastureParams,
                         ui->spinPastureGenScale->value(),
                         ui->spinPastureGenSmothIt->value(),
                         this );

    mMeadow->setGrowing(ui->checkGrowingMeadow->isChecked());

    // Generate herd
    mHerd = new Herd(isSimulation(), mMeadow);

    if( isLoad ) {
        // Load from stored file
        // generate only random position and the medow
        if( ! mHerd->load(dir + (isSimulation() ? ANIMALS_LIST_FILE_SIM : ANIMALS_LIST_FILE),
                    ui->doubleSpinArea->value(),
                    ui->doubleSpinAnimalSize->value(),
                    ui->spinAnimalGrazingCapacity->value() ) ) {
            return false;
        }

    }else {
        // generate from settings
        if( ! mHerd->generate( ui->spinAnimalsCount->value(),
                        ui->doubleSpinArea->value(),
                        ui->spinCollarsPercentage->value(),
                        ui->spinBolusesPercentage->value(),
                        ui->spinMalesPercentage->value(),
                        ui->doubleSpinAnimalSize->value(),
                        ui->spinAnimalGrazingCapacity->value()) ) {
            return false;
        }

    }

    mNetwork = new Network( mSettings, ui->spinGateways->value(),  ui->doubleSpinArea->value());

    if( !syncDevices()) {
        return false;
    }



    // create scene
    mScene->create(mSceneView, mHerd, mNetwork,
                   mMeadow->dim(),
                   mHerd->collarsCount() * mHerd->count(),
                   mHerd->collarsCount() * mNetwork->gatewaysCount() );
    mScene->update(mHerd, mMeadow, mNetwork, true, INITIAL_HERD_SPREAD);

    mSceneView->setMeadow(mMeadow);

    ui->table->setColumnCount(TABLE_COLS_COUNT);
    ui->table->setRowCount(mHerd->count() );

    QFont boldFont;
    boldFont.setBold(true);

    ui->table->clear();
    for (int row = 0; row < mHerd->count(); row++) {
        Animal* a = mHerd->animal(row);
        QTableWidgetItem *item = new QTableWidgetItem();

        // Set bold text and background color
        if( a->hasCollar() ) {
            item->setFont(boldFont);
        }
        ui->table->setItem(row, 0, item);

        item = new QTableWidgetItem(QString(""));
        if( a->hasCollar() ) { item->setFont(boldFont); }
        ui->table->setItem(row, 1, item);

        item = new QTableWidgetItem(QString(""));
        if( a->hasCollar() ) { item->setFont(boldFont); }
        ui->table->setItem(row, 2, item);
    }

    QStringList hHeader;
    hHeader << "Seen by" << "Seeing" << "Readings";
    ui->table->setHorizontalHeaderLabels(hHeader);


    // Set stretch factors (percentages)
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //int w = ui->table->horizontalHeader()->width();
    //ui->table->horizontalHeader()->resizeSection(0, w*0.66f); //
    //ui->table->horizontalHeader()->resizeSection(1, w*0.33f); //
    //ui->table->horizontalHeader()->resizeSection(2, w*0.33f); //

    // Auto resize columns and rows to content
    // ui->table->resizeColumnsToContents();
    ui->table->resizeRowsToContents();


    // ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->checkShepard->setEnabled(true);
    ui->checkRecursiveCollision->setEnabled(true);

    mDevMsg->updateDevices();

    ui->actionSave->setEnabled(true);

    mScene->loadFence();

    ui->progressFence->setValue(0);
    ui->checkFence->setChecked(false);
    ui->checkFence->setToolTip("Press to activate the fence.");

    mScene->showPopup("Scene created!");

    mIsCreated = true;

    if( !isLoad ) {
        mHerd->storeAnimals();
    }

    mDlgCollar->init(mHerd->animalsWithCollars());

    return true;
}


void MainWindow::onUpdate()
{
    if( ui->btnPause->isChecked() ) {
        return;
    }

    gSimTimer->update();

    mDlgCollar->update();

    if( !mIsCreated) {
        return;
    }

    mMeadow->update(gSimTimer->tickSeconds());

    ui->editInfo->setText( QString("Food:%1%").arg( mMeadow->kgRatio(100), 0, 'f', 2) );

    ui->btnPause->setText(QString("%1:%2:%3")
                              .arg(gSimTimer->hours(), 2, 10, '0')
                              .arg(gSimTimer->minutes(), 2, 10, '0')
                              .arg(gSimTimer->seconds(), 2, 10, '0'));

    QPointF attractor = mSceneView->rightPos();

    if( isSimulation() ) {
        mHerd->updateSimulation( gSimTimer->tickSeconds(),
                      mSceneView->isRightPress() ? &attractor : nullptr,
                      ui->checkRecursiveCollision->isChecked(),
                      ui->spinAttrPower ->value(),
                      ui->spinAttrDist ->value(),
                      ui->spinRepDist ->value(),
                      ui->spinCollDist ->value(),
                      ui->spinMaxSpeed ->value(),
                      ui->spinFriction ->value(),
                      ui->spinRotFad ->value(),
                      ui->spinTransDist ->value(),
                      qDegreesToRadians(ui->spinTransAngle->value())
                      );
    } else {
        mHerd->updateReal();
    }

    mNetwork->update(mHerd, MAX_COLLAR_GATEWAY_DISTANCE );

    mScene->update(mHerd, mMeadow, mNetwork);
    // mSceneView->invalidateScene();

    for (int row = 0; row < mHerd->count(); row++) {
        Animal* animal = mHerd->animal(row);

        QTableWidgetItem *item = ui->table->item(row, 0);
        item->setText( QString("%1").arg(animal->observersCount()) );
        // set number of observers
        item = ui->table->item(row, 1);
        item->setText( QString("%1").arg(animal->observingCount()) );

        item = ui->table->item(row, 2);
        item->setText( QString("%1").arg(animal->readings()) );

        // set the color connected to number of readigs
        // item->setBackground(QBrush(QColor(220, 240, 255))); // light blue
    }

    if( mIsFenceSetup ) {
        bool isActivate = ui->checkFence->isChecked();
        int count = mDevManager->getDevicesFenceStatus(isActivate);
        ui->progressFence->setValue(count);
        if( count >= mHerd->collarsCount()) {
            mIsFenceSetup = false;
            ui->checkFence->setTitle("Fence");
            mScene->fenceActivate(isActivate);
        }
    }
}




void MainWindow::onRowClicked(int row, int column)
{
    (void)column;
    mScene->selectAnimalItem(row);
}


void MainWindow::on_checkShepard_toggled(bool checked)
{
    if( mHerd ) {
        mHerd->activateShepherd(checked);
    }
}

void MainWindow::on_checkParamsHerding_toggled(bool checked)
{
    ui->widgetSim->setVisible(checked);
    ui->scrollAreaParamsWidget->adjustSize();
    ui->scrollAreaParamsWidget->setMinimumSize(ui->scrollAreaParamsWidget->sizeHint());
}


void MainWindow::on_checkParamsG_toggled(bool checked)
{
    ui->widgetGrazing->setVisible(checked);
    ui->scrollAreaParamsWidget->adjustSize();
    ui->scrollAreaParamsWidget->setMinimumSize(ui->scrollAreaParamsWidget->sizeHint());
}

void MainWindow::moveEvent(QMoveEvent *)
{
}

void MainWindow::resizeEvent(QResizeEvent *)
{
}



void MainWindow::setStatus(const QString &txt)
{
    statusBar()->showMessage(txt);
}

void MainWindow::onDeviceMessage(const QString &devEUI, const QJsonObject &jobjResponse)
{
    mDevMsg->onResponse(devEUI, jobjResponse);
}

// Called when all the devices are configured with addresses
void MainWindow::onDevicesReady(bool isStore )
{
    if( isStore ) {
        mHerd->storeDevices();
    }
}

/*
void MainWindow::onMqttConnected()
{
    if( mIsLoadLast ) {
        create(true);
    }
}

*/
void MainWindow::errorMsgBox(const QString &msg)
{
    QMessageBox::critical(this, "Error", msg);
}

void MainWindow::infoMsgBox(const QString &msg)
{
    QMessageBox::information(this, "Info", msg);
}

bool MainWindow::question(const QString &msg)
{
    return QMessageBox::Yes == QMessageBox::question(this, "Question?", msg);
}

void MainWindow::onSceneItemSelected()
{
    if( !mScene->selectedAnimal() ) {
        return;
    }

    Animal* a = mScene->selectedAnimal()->animal();
    if( a && a->hasCollar() ) {
        mDevMsg->selectCollarByEUI( a->collar()->eui() );
    }
}


void MainWindow::onError(const QString &err)
{
    setStatus(err);
    if( mConsole->isVisible() ) {
        mConsole->setFocus();
    }

    if( mScene ) {
        mScene->showPopup("Critical errors! Open the console!");
    }
}

void MainWindow::onConsoleClose()
{
    ui->actionConsole->setChecked(false);
}

void MainWindow::onDeviceMsgClose()
{
    ui->actionDeviceMsg->setChecked(false);
}

void MainWindow::onDlgCollarClose()
{
    ui->actionDlgCollar->setChecked(false);
}

void MainWindow::onDlgSettingsChanged()
{
    setStatus("Settings changed. Applying...");
    qInfo() << "";
    // TODO: reread all UI that uses settings
    //
}

void MainWindow::on_btnLoad_clicked()
{
    create(true);
}


void MainWindow::on_actionConsole_toggled(bool arg1)
{
    mConsole->setVisible(arg1);
}


void MainWindow::on_actionDeviceMsg_toggled(bool arg1)
{
    mDevMsg->setVisible(arg1);
}


void MainWindow::on_btnRefill_clicked()
{
    mMeadow->refill();
}


void MainWindow::on_checkGrowingMeadow_toggled(bool checked)
{
    if( mMeadow ) {
        mMeadow->setGrowing(checked);
    }
}


void MainWindow::on_btnShowInfo_toggled(bool checked)
{
    ui->groupFold->setVisible(checked);
    ui->btnShowInfo->setText(checked ? ">" : "<");
}


void MainWindow::on_btnCopyCenter_clicked()
{
    QGeoCoordinate location(ui->spinCenterLat->value(), ui->spinCenterLong->value());
    QApplication::clipboard()->setText(location.toString());
}


void MainWindow::on_actionSave_triggered()
{
    QString dirStr;
    bool isChoice = false;

    while(!isChoice) {
        QString choice = QFileDialog::getSaveFileName(this,
                                                      "Choose save",
                                                      SAVE_DIR, QString(), nullptr,
                                                      QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
        if( choice.isEmpty() ) {
            return;
        }

        dirStr = choice + "/";

        if( QFile::exists(dirStr) ) {
            auto result = QMessageBox::question(this, "Rewrite save?",
                                "Save already exists. Do you to overwrite it?",
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            switch (result) {
            case QMessageBox::Yes:
                isChoice = true;
                return;
            case QMessageBox::No:
                break;
            default:
                return;
            }
        }else{
            QDir dir(SAVE_DIR);
            if(!dir.mkdir(choice)) {
                qCritical() << "Error creating directory" << choice << "is" << SAVE_DIR;
            }else{
                isChoice = true;
            }
        }
    }

    mHerd->storeAnimals(dirStr);
    mHerd->storeDevices(dirStr);
}


void MainWindow::on_actionLoad_triggered()
{
    QString choice = QFileDialog::getExistingDirectory(this,
                                                  "Choose load",
                                                  SAVE_DIR);

    if( choice.isEmpty() ) {
        return;
    }

    QString dirStr = choice + "/";
    if( create(true, dirStr) ) {
        setStatus(QString("Loaded from %1").arg(dirStr));
    }else{
        setStatus(QString("Error loading from %1").arg(dirStr));
    }
}


void MainWindow::on_btnUnitTest_clicked()
{
    mScene->storeImage();
}


void MainWindow::on_checkPastureGenParams_toggled(bool checked)
{
    ui->widgetPastureGen->setVisible(checked);
    ui->scrollAreaParamsWidget->adjustSize();
    ui->scrollAreaParamsWidget->setMinimumSize(ui->scrollAreaParamsWidget->sizeHint());
}


void MainWindow::on_checkFenceAdd_checkStateChanged(const Qt::CheckState &state)
{
    if( state == Qt::Checked ) {
        mSceneView->setMode(SceneView::Mode::Fence);
    }else {
        mSceneView->setMode(SceneView::Mode::Explore);
    }

    updateFenceButtons();
}


void MainWindow::on_btnFenceRemoveLast_clicked()
{
    mScene->fenceRemove();
}



void MainWindow::updateFenceButtons()
{
    if( ui->checkFenceAdd->isChecked() ) {
        ui->btnFenceRemoveLast->setEnabled(mScene->fencePointsCount());
        ui->btnFenceClear->setEnabled(mScene->fencePointsCount() < VIRTUAL_FENCE_MAX_POINTS);
    }else {
        ui->btnFenceRemoveLast->setEnabled(false);
        ui->btnFenceClear->setEnabled(false);
    }
}

bool MainWindow::syncDevices()
{
    if( !mDevManager->syncDevices( mHerd->jsonAnimalsList(true).toUtf8(), mHerd->gatherDevices(), mNetwork->edge() ) ) {
        errorMsgBox("MainWindow: Error sync devices! See console.");
        return false;
    }

    return true;
}

void MainWindow::on_btnPause_toggled(bool checked)
{
    ui->btnPause->setToolTip( checked ? "Unpause simulation" : "Pause simulation");
}


void MainWindow::on_actionScene_UI_toggled(bool is)
{
    mScene->showUI(is);
}


void MainWindow::on_actionReset_triggered()
{
    if( mScene ) {
        mScene->resetView();
    }
}



void MainWindow::on_checkFence_toggled(bool is)
{
    if( !mIsCreated) {
        return;
    }

    ui->progressFence->setValue(0);

    QVector<QGeoCoordinate> fenceGeoPoints;

    if( is ) {
        ui->checkFenceAdd->setEnabled(false);
        fenceGeoPoints = mScene->fenceGepPoints(mMeadow);
        ui->checkFence->setToolTip("Click to deactivate the fence");
        ui->checkFence->setTitle("Fence (activating)");
    }else
    {
        ui->checkFenceAdd->setEnabled(true);
        ui->checkFence->setToolTip("Click to activate the fence");
        ui->checkFence->setTitle("Fence (deactivating)");
    }

    ui->progressFence->setMinimum(0);
    ui->progressFence->setMaximum(mDevManager->collarsCount());
    ui->progressFence->setEnabled(true);
    updateFenceButtons();

    mIsFenceSetup = true;
    mDevManager->setupFence(mMeadow->geoCenter(), fenceGeoPoints);
}

/*
void MainWindow::on_btnClearCount_clicked()
{
    if( gSimTools->fileExists(ANIMALS_LIST_FILE) ) {
        if( QMessageBox::Yes != QMessageBox::question(this, "Clear herd?", "This will erase existing saved animals list! Proceed with clearing the herd?") ) {
            return;
        }
    }

    ui->spinAnimalsCount->setValue(0);

    create(false);
}
*/

// Add device (animal)
void MainWindow::on_btnAdd_clicked()
{
    if( nullptr == mHerd ) {
        errorMsgBox( "Cannot add animal - herd not available. Load or generate!" );
        return;
    }

    bool isPause = ui->btnPause->isChecked();

    if( !isPause) {
        ui->btnPause->setChecked(true);
    }

    DialogRegisterAnimal dlg(mHerd, this);
    dlg.exec();

    if( !isPause) {
        ui->btnPause->setChecked(false);
    }

    /*
    if( dlg.devicesChanged()) {
        mDevMsg->updateDevices();
        syncDevices();
        mHerd->storeAnimals();
    }*/

}

void MainWindow::on_actionDlgCollar_triggered()
{
    mDlgCollar->setVisible(true);
}


void MainWindow::on_actionSettings_triggered()
{
    mDlgSettings->exec();
}


bool MainWindow::registerCollar(Animal *animal, const QString &euiHex, const QString &akeyHex, const QString &nkeyHex)
{

    mDevMsg->updateDevices();
    syncDevices();
    mHerd->storeAnimals();
}

void MainWindow::reload()
{
    create(true);
}

void MainWindow::on_spinFemalesCount_valueChanged(int)
{
    if( mCountPercentageRecalc ) {
        return;
    }
    int count = ui->spinAnimalsCount->value();
    int fc = ui->spinFemalesCount->value();

    if( fc > count) {
        count = fc;
        ui->spinAnimalsCount->setValue(count);
    }


    int mc = count - fc;
    int fp = 100 * fc / count;
    int mp = 100 - fp;
    mCountPercentageRecalc = true;
    ui->spinMalesCount->setValue(mc);
    ui->spinMalesPercentage->setValue(mp);
    ui->spinFemalesPercentage->setValue(fp);
    mCountPercentageRecalc = false;
}


void MainWindow::on_spinMalesCount_valueChanged(int )
{
    if( mCountPercentageRecalc ) {
        return;
    }
    int count = ui->spinAnimalsCount->value();
    int mc = ui->spinMalesCount->value();

    if( mc > count) {
        count = mc;
        ui->spinAnimalsCount->setValue(count);
    }


    int fc = count - mc;
    int mp = 100 * mc / count;
    int fp = 100 - mp;
    mCountPercentageRecalc = true;
    ui->spinFemalesCount->setValue(fc);
    ui->spinFemalesPercentage->setValue(fp);
    ui->spinMalesPercentage->setValue(mp);
    mCountPercentageRecalc = false;
}


void MainWindow::on_spinMalesPercentage_valueChanged(int )
{
    if( mCountPercentageRecalc ) {
        return;
    }
    int count = ui->spinAnimalsCount->value();
    int mp = ui->spinMalesPercentage->value();
    int fp = 100 - mp;
    int mc = mp * count / 100;
    int fc = count - mc;
    mCountPercentageRecalc = true;
    ui->spinFemalesPercentage->setValue(fp);
    ui->spinFemalesCount->setValue(fc);
    ui->spinMalesCount->setValue(mc);
    mCountPercentageRecalc = false;
}


void MainWindow::on_spinFemalesPercentage_valueChanged(int )
{
    if( mCountPercentageRecalc ) {
        return;
    }
    int count = ui->spinAnimalsCount->value();
    int fp = ui->spinFemalesPercentage->value();
    int mp = 100 - fp;
    int fc = fp * count / 100;
    int mc = count - fc;
    mCountPercentageRecalc = true;
    ui->spinMalesPercentage->setValue(mp);
    ui->spinFemalesCount->setValue(fc);
    ui->spinMalesCount->setValue(mc);
    mCountPercentageRecalc = false;
}

void MainWindow::on_spinAnimalsCount_valueChanged(int count)
{
    if( mCountPercentageRecalc ) {
        return;
    }

    int fp = ui->spinFemalesPercentage->value();
    int mp = 100 - fp;
    int fc = count * fp / 100;
    int mc = count - fc;

    mCountPercentageRecalc = true;
    ui->spinMalesPercentage->setValue(mp);
    ui->spinFemalesPercentage->setValue(fp);
    ui->spinFemalesCount->setValue(fc);
    ui->spinMalesCount->setValue(mc);
    mCountPercentageRecalc = false;
}


void MainWindow::on_radioAnimalsCount_toggled(bool checked)
{
    if( mCountPercentageToggled ) {
        return;
    }

    mCountPercentageToggled = true;
    ui->spinMalesCount->setEnabled(checked);
    ui->spinFemalesCount->setEnabled(checked);
    ui->spinMalesPercentage->setEnabled(!checked);
    ui->spinFemalesPercentage->setEnabled(!checked);
    mCountPercentageToggled = false;
}


void MainWindow::on_radioAnimalsPercentage_toggled(bool checked)
{
    if( mCountPercentageToggled ) {
        return;
    }
    mCountPercentageToggled = true;
    ui->spinMalesCount->setEnabled(!checked);
    ui->spinFemalesCount->setEnabled(!checked);
    ui->spinMalesPercentage->setEnabled(checked);
    ui->spinFemalesPercentage->setEnabled(checked);
    mCountPercentageToggled = false;
}



