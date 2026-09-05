#include "defines.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

void MainWindow::initSimulation()
{
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

}
