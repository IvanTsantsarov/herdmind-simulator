#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scene.h"
#include "sceneview.h"
#include "herd.h"
#include "defines.h"
#include "animal.h"


#define TABLE_COLS_COUNT 3

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create scene
    mScene = new Scene(this);
    mScene->setSceneRect(-INITIAL_MEDDOW_SIZE, -INITIAL_MEDDOW_SIZE, 2*INITIAL_MEDDOW_SIZE, 2*INITIAL_MEDDOW_SIZE);

    mSceneView = new SceneView(mScene, this);
    ui->mainVerticalLayout->addWidget(mSceneView);
    mSceneView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    /// Table initialization
    ui->table->setColumnCount(TABLE_COLS_COUNT);

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
    ui->spinAttrDist ->setValue(ANIMAL_ATTRACTION_DISTANCE);
    ui->spinRepDist ->setValue(ANIMAL_REPELING_DISTANCE);
    ui->spinCollDist ->setValue(ANIMAL_COLLIDING_DISTANCE);
    ui->spinMaxSpeed ->setValue(ANIMAL_MAX_SPEED);
    ui->spinFriction  ->setValue(ANIMAL_FRICTION);
    ui->spinRotFad ->setValue(ANIMAL_ROTATION_FADING);
    ui->spinTransDist ->setValue(BOLUS_TRANSMIT_DISTANCE);
    ui->spinTransAngle ->setValue(BOLUS_TRANSMIT_ANGLE);

    QObject::connect(&mUpdateTimer, &QTimer::timeout, this, &MainWindow::onUpdate );
    mUpdateTimer.start(HERD_UPDATE_INTERVAL);

    // Connect the cell click signal to your custom slot
    connect(ui->table, &QTableWidget::cellClicked,
            this, &MainWindow::onRowClicked);

    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onUpdate()
{
    if( !mHerd) {
        return;
    }

    QPointF attractor = mSceneView->rightPos();

    mHerd->update(mSceneView->isRightPress() ? &attractor : nullptr,
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
    mScene->update(mHerd);

    for (int row = 0; row < mHerd->count(); row++) {
        Animal* a = mHerd->animal(row);

        QTableWidgetItem *item = ui->table->item(row, 0);
        item->setText( QString("%1").arg(a->observersCount()) );
        // set number of observers
        item = ui->table->item(row, 1);
        item->setText( QString("%1").arg(a->observingCount()) );

        item = ui->table->item(row, 2);
        item->setText( QString("%1").arg(0) );

        // set the color connected to number of readigs
        // item->setBackground(QBrush(QColor(220, 240, 255))); // light blue
    }
}

void MainWindow::on_btnGenerate_clicked()
{
    // create herd
    if( mHerd) {
        delete mHerd;
        mHerd = nullptr;
    }

    mHerd = new Herd();
    mHerd->generate( ui->spinAnimalsCount->value(),
                     ui->doubleSpinAnimalSize->value(),
                     ui->doubleSpinArea->value(),
                     ui->spinCollarsPercentage->value() );

    // create scene
    mScene->create( ui->spinAnimalsCount->value(), mHerd->collarsCount() * mHerd->count() );
    mScene->update(mHerd, true, INITIAL_HERD_SPREAD);

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
    hHeader << "Pos" << "Mas" << "Sl";
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
}

void MainWindow::onRowClicked(int row, int column)
{
    mScene->selectFigure(row);
}

