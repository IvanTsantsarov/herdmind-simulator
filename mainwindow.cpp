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
#include "focusanim.h"
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

#define TABLE_COLS_COUNT 3
#define REMINDER_DELAY 3000

MainWindow* gMainWindow = nullptr;

MainWindow::MainWindow(QSettings &env, const QSettings &settings, QWidget *parent)
    : QMainWindow(parent), mEnv(env), mSettings(settings)
    , ui(new Ui::MainWindow)
{
    mConsole = new DialogConsole(settings, this);
    gMainWindow = this;

    ui->setupUi(this);

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


    // Generation parameters
    ui->doubleSpinAnimalSize->setValue(ANIMAL_LENGTH);
    ui->spinAnimalsCount->setValue(INITIAL_ANIMALS_COUNT);
    ui->doubleSpinArea->setValue(INITIAL_HERD_SPREAD);
    ui->spinCollarsPercentage->setValue(INITIAL_COLLAR_PERCENTAGE);

    // Simulation parameters
    ui->spinAttrPower->setValue(ANIMAL_ATTRACTION_POWER);
    ui->spinAttrDist->setValue(ANIMAL_ATTRACTION_DISTANCE);
    ui->spinRepDist->setValue(ANIMAL_REPELING_DISTANCE);
    ui->spinCollDist->setValue(ANIMAL_COLLIDING_DISTANCE);
    ui->spinMaxSpeed->setValue(ANIMAL_MAX_SPEED);
    ui->spinFriction->setValue(ANIMAL_FRICTION);
    ui->spinRotFad->setValue(ANIMAL_ROTATION_FADING);
    ui->spinTransDist->setValue(BOLUS_TRANSMIT_DISTANCE);
    ui->spinTransAngle->setValue(BOLUS_TRANSMIT_ANGLE);

    // Grazing parameters
    ui->spinMeadowDimX->setValue(MEADOW_INITIAL_DIM_WIDTH);
    ui->spinMeadowDimY->setValue(MEADOW_INITIAL_DIM_HEIGHT);
    ui->spinMeadowCapacity->setValue(MEADOW_INITIAL_CAPACITY);
    ui->spinMeadowGrowingSpeed->setValue(MEADOW_GROWING_SPEED);
    ui->spinAnimalGrazingCapacity->setValue(ANIMAL_INITIAL_GRAZING_CAPACITY);
    ui->spinLawnRadius->setValue(LAWN_RADIUS);
    ui->spinAnimalsPerLawn->setValue(LAWN_ANIMALS_MAX_COUNT);

    ui->spinCenterLong->setValue(FARM_INITIAL_LOCATION_LONG);
    ui->spinCenterLat->setValue(FARM_INITIAL_LOCATION_LAT);

    ui->spinPastureGenRadius->setValue(PASTURE_GEN_RADIUS);
    ui->spinPastureGenScale->setValue(PASTURE_GEN_SCALE);
    ui->spinPastureGenCount->setValue(PASTURE_GEN_COUNT);
    ui->spinPastureGenSmothIt->setValue(PASTURE_GEN_SMOOTH_IT);
    ui->spinPastureGenAmpMin->setValue(PASTURE_GEN_AMP_MIN);
    ui->spinPastureGenAmpMax->setValue(PASTURE_GEN_AMP_MAX);
    ui->spinPastureGenWaveMin->setValue(PASTURE_GEN_WAVE_MIN);
    ui->spinPastureGenWaveMax->setValue(PASTURE_GEN_WAVE_MAX);


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

    // Create focus animation
    mFocusAnim = new FocusAnim(this);

    // Setup click reminder timer to show focus animation when needed
    mReminder = new QTimer(this);
    connect(mReminder, &QTimer::timeout, this, &MainWindow::onConnectReminger );
    mReminder->start( REMINDER_DELAY );


    // try to load saved properties
    if( gSimTools->fileExists(ANIMALS_LIST_FILE)) {
        qInfo() << "Animal list file available:" << ANIMALS_LIST_FILE;
        ui->btnLoad->setEnabled(true);
    }else {
        qInfo() << "File not exists:" << ANIMALS_LIST_FILE;
        ui->btnLoad->setEnabled(false);
    }

    mDevManager = new DevManager(settings);

    mDevMsg = new DialogDeviceMsg(mDevManager, this);

    if( settings.value("GUI/isConsole").toBool() ) {
        ui->actionConsole->setChecked(true);
    }

    if( settings.value("GUI/isDeviceMsg").toBool() ) {
        ui->actionDeviceMsg->setChecked(true);
    }

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
    mEnv.setValue("UI/GroupFold", ui->btnShowInfo->isChecked());
    mEnv.setValue("UI/isGrowing", ui->checkGrowingMeadow->isChecked());
    mEnv.setValue("UI/DebugInfo", mConsole->isDebugInfo());
    mScene->saveFence();
}


bool MainWindow::create(bool isLoad, const QString& dir)
{
    mIsCreated = false;

    mFocusAnim->stop();

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

    // Generate herd
    mHerd = new Herd();

    if( isLoad ) {
        // Load from stored file
        // generate only random position and the medow
        if( ! mHerd->load(dir + ANIMALS_LIST_FILE,
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
                        ui->spinMalesPercentage->value(),
                        ui->doubleSpinAnimalSize->value(),
                        ui->spinAnimalGrazingCapacity->value()) ) {
            return false;
        }

    }

    mNetwork = new Network( mSettings, ui->spinGateways->value(),  ui->doubleSpinArea->value());

    mDevManager->syncDevices( mHerd->jsonAnimalsList(true).toUtf8(), mHerd->gatherDevices(), mNetwork->edge() );

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
        if( a->hasCollar() ) { item->setFont(boldFont); }
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

    mIsCreated = true;
    ui->actionSave->setEnabled(true);

    mScene->loadFence();

    mScene->showPopup("Scene created!");

    return true;
}


void MainWindow::onUpdate()
{
    gSimTimer->update();

    if( !mIsCreated) {
        return;
    }

    mMeadow->update(gSimTimer->tickSeconds());

    ui->editInfo->setText( QString("Food:%1%").arg( mMeadow->kgRatio(100), 0, 'f', 2) );

    ui->editTime->setText(QString("%1:%2:%3")
                              .arg(gSimTimer->hours(), 2, 10, '0')
                              .arg(gSimTimer->minutes(), 2, 10, '0')
                              .arg(gSimTimer->seconds(), 2, 10, '0'));

    QPointF attractor = mSceneView->rightPos();

    mHerd->update( gSimTimer->tickSeconds(),
                  mMeadow,
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

}


void MainWindow::on_btnGenerate_clicked()
{
    if( gSimTools->fileExists(ANIMALS_LIST_FILE) ) {
        if( QMessageBox::Yes != QMessageBox::question(this, "Generate new herd?", "This will erase existing saved animals list! Proceed with generating?") ) {
            return;
        }
    }

    create(false);
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
    if( mFocusAnim ) {
        mFocusAnim->stop();
    }
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    if( mFocusAnim ) {
        mFocusAnim->stop();
    }
}

void MainWindow::onConnectReminger()
{
    if( !mHerd ) {
        mFocusAnim->start(gSimTools->fileExists(ANIMALS_LIST_FILE) ? ui->btnLoad : ui->btnGenerate);
    }
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
        mHerd->storeLists();
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

bool MainWindow::question(const QString &msg)
{
    return QMessageBox::Yes == QMessageBox::question(this, "Question?", msg);
}

void MainWindow::onError(const QString &err)
{
    setStatus(err);
    if( mConsole->isVisible() ) {
        mConsole->setFocus();
    }

    mScene->showPopup("Critical errors! Open the console!");
}

void MainWindow::onConsoleClose()
{
    ui->actionConsole->setChecked(false);
}

void MainWindow::onDeviceMsgClose()
{
    ui->actionDeviceMsg->setChecked(false);
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

    if( mHerd->storeLists(dirStr) ) {
        setStatus(QString("Saved in %1").arg(dirStr));
    }else{
        setStatus(QString("Error saving in %1").arg(dirStr));
    }
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
}


void MainWindow::on_btnFenceRemoveLast_clicked()
{
    mScene->fenceRemove();
}


void MainWindow::on_checkFenceActive_checkStateChanged(const Qt::CheckState &state)
{
    if( mIsAskingForFence ) {
        return;
    }

    QVector<QGeoCoordinate> fenceGeoPoints;
    mIsAskingForFence = true;

    if( Qt::Checked == state ) {
        if( question("Activate the fence?") ) {
            ui->checkFenceActive->setText("Activating...");
            ui->checkFenceAdd->setEnabled(false);
            ui->btnFenceRemoveLast->setEnabled(false);
            ui->btnFenceClear->setEnabled(false);
            fenceGeoPoints = mScene->fenceGepPoints(mMeadow);
        }else{
            ui->checkFenceActive->setChecked(false);
            return;
        }
    }else
    if( Qt::Unchecked == state ) {
        if( question("Deativate the fence?") ) {
            ui->checkFenceActive->setText("Deactivating...");
        }else{
            ui->checkFenceActive->setChecked(true);
            return;
        }
    }else {
        return;
    }

    ui->progressFence->setMinimum(0);
    ui->progressFence->setMaximum(mDevManager->devicesCount());
    ui->progressFence->setValue(0);
    ui->progressFence->setEnabled(true);

    mDevManager->setupFence(fenceGeoPoints);

    mIsAskingForFence = false;
}

