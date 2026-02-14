#include <QSettings>
#include <QFile>
#include "mainwindow.h"
#include "focusanim.h"
#include "ui_mainwindow.h"
#include "scene.h"
#include "sceneview.h"
#include "herd.h"
#include "defines.h"
#include "animal.h"
#include "network.h"
#include "simtools.h"

#define SETTINGS_NAME "settings.ini"
#define TABLE_COLS_COUNT 3
#define REMINDER_DELAY 3000

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if( !QFile::exists(SETTINGS_NAME) ) {
        QFile::copy("://" SETTINGS_NAME, SETTINGS_NAME);
    }

    Q_ASSERT( QFile::exists(SETTINGS_NAME) );

    QSettings settings(SETTINGS_NAME, QSettings::IniFormat);

    mTools = new SimTools(settings);

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
    ui->spinMeadowRadius->setValue(MEADOW_INITIAL_RADIUS);
    ui->spinMeadowCapacity->setValue(MEADOW_INITIAL_CAPACITY);
    ui->spinAnimalGrazingCapacity->setValue(ANIMAL_INITIAL_GRAZING_CAPACITY);
    ui->spinLawnRadius->setValue(LAWN_RADIUS);
    ui->spinAnimalsPerLawn->setValue(LAWN_ANIMALS_MAX_COUNT);

    ui->spinCenterLong->setValue(FARM_INITIAL_LOCATION_LONG);
    ui->spinCenterLat->setValue(FARM_INITIAL_LOCATION_LAT);

    QObject::connect(&mUpdateTimer, &QTimer::timeout, this, &MainWindow::onUpdate );
    mUpdateTimer.start(HERD_UPDATE_INTERVAL);

    // Connect the cell click signal to your custom slot
    connect(ui->table, &QTableWidget::cellClicked,
            this, &MainWindow::onRowClicked);


    ui->widgetSim->setVisible(false);
    ui->widgetGrazing->setVisible(false);
    showMaximized();

    // ui->scrollAreaParams->setWidgetResizable(false); // chatGPT was wrong about this

    // Create focus animation
    mFocusAnim = new FocusAnim(this);

    // Setup click reminder timer to show focus animation when needed
    mReminder = new QTimer(this);
    connect(mReminder, &QTimer::timeout, this, &MainWindow::onConnectReminger );
    mReminder->start( REMINDER_DELAY );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnGenerate_clicked()
{
    mFocusAnim->stop();

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
    mHerd->generate( ui->spinAnimalsCount->value(),
                    ui->doubleSpinArea->value(),
                    ui->spinCollarsPercentage->value(),
                    ui->doubleSpinAnimalSize->value(),
                    ui->spinAnimalGrazingCapacity->value()
                    );

    // Generate meadow
    mMeadow = new Meadow(QPoint(0, 0),
                         QSize( ui->spinMeadowRadius->value() * 2, ui->spinMeadowRadius->value() * 2),
                         ui->spinLawnRadius->value(),
                         ui->spinMeadowCapacity->value(),
                         ui->spinAnimalsPerLawn->value()
                         );

    mNetwork = new Network( ui->spinGateways->value(),  ui->doubleSpinArea->value());

    // create scene
    mScene->create(mSceneView, mHerd, mNetwork,
                   ui->spinAnimalsCount->value(), mMeadow->lawnsCount(), mHerd->collarsCount() * mHerd->count(),  mHerd->collarsCount() * mNetwork->gatewaysCount() );
    mScene->update(mHerd, mMeadow, mNetwork, true, INITIAL_HERD_SPREAD);

    ui->table->setColumnCount(TABLE_COLS_COUNT);
    ui->table->setRowCount(ui->spinAnimalsCount->value());

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

    mHerdTimer.start();

    ui->checkShepard->setEnabled(true);
    ui->checkRecursiveCollision->setEnabled(true);
}

void MainWindow::onUpdate()
{
    if( !mHerd) {
        return;
    }

    // display elapsed time from starting the simulation
    int seconds = mHerdTimer.elapsed() / 1000;
    int minutes = seconds / 60;
    int hours = seconds / 3600;
    int minutesInHours = hours * 60;
    minutes -= minutesInHours;
    seconds -= (minutesInHours + minutes)*60;

    mMeadow->update();

    ui->editInfo->setText( QString("Food:%1%").arg( mMeadow->kgRatio(100), 0, 'f', 2) );

    ui->editTime->setText(QString("%1:%2:%3")
                              .arg(hours, 2, 10, '0')
                              .arg(minutes, 2, 10, '0')
                              .arg(seconds, 2, 10, '0'));

    QPointF attractor = mSceneView->rightPos();

    mHerd->update( mHerdTimer.elapsed(),
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
        mFocusAnim->start(ui->btnGenerate);
    }
}
