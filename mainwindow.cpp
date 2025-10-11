#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scene.h"
#include "sceneview.h"
#include "herd.h"
#include "defines.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create herd
    mHerd = new Herd();
    mHerd->generate( ANIMAL_LENGTH, INITIAL_ANIMALS_COUNT, INITIAL_HERD_SPREAD, INITIAL_COLLAR_PERCENTAGE);

    // create scene
    mScene = new Scene(this);
    mScene->setSceneRect(-INITIAL_MEDDOW_SIZE, -INITIAL_MEDDOW_SIZE, 2*INITIAL_MEDDOW_SIZE, 2*INITIAL_MEDDOW_SIZE);
    mScene->create( INITIAL_ANIMALS_COUNT, mHerd->collarsCount() * mHerd->count() );
    mScene->update(mHerd, true, INITIAL_HERD_SPREAD);

    mSceneView = new SceneView(mScene, this);
    ui->mainVerticalLayout->addWidget(mSceneView);
    mSceneView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QObject::connect(&mUpdateTimer, &QTimer::timeout, this, &MainWindow::onUpdate );
    mUpdateTimer.start(HERD_UPDATE_INTERVAL);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onUpdate()
{
    QPointF attractor = mSceneView->rightPos();
    mHerd->update(mSceneView->isRightPress() ? &attractor : nullptr,
                  ANIMAL_ATTRACTION_POWER,
                  ANIMAL_ATTRACTION_DISTANCE,
                  ANIMAL_REPELING_DISTANCE,
                  ANIMAL_COLLIDING_DISTANCE,
                  ANIMAL_MAX_SPEED,
                  ANIMAL_MAX_FRICTION,
                  ANIMAL_ROTATION_FADING,
                  BOLUS_TRANSMIT_DISTANCE,
                  qDegreesToRadians(BOLUS_TRANSMIT_ANGLE)
                  );
    mScene->update(mHerd);
}
