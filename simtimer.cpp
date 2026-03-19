#include <QElapsedTimer>

#include "simtimer.h"

SimTimer* gSimTimer = nullptr;

SimTimer::SimTimer(QObject *parent)
    : QObject{parent}
{
    gSimTimer = this;
    mElapsedTimer = new QElapsedTimer;
    mElapsedTimer->start();
}

SimTimer::~SimTimer()
{
    delete mElapsedTimer;
    mElapsedTimer = nullptr;
}

void SimTimer::update()
{
    // display elapsed time from starting the simulation
    mMillis = mElapsedTimer->elapsed();
    mSeconds =  mMillis / 1000;
    mMinutes = mSeconds / 60;
    mHours = mSeconds / 3600;
    mDays = mHours / 24;
    quint64 minutesInHours = mHours * 60;
    mMinutes -= minutesInHours;
    mSeconds -= (minutesInHours + mMinutes)*60;

    mMillisTick = mMillis - mLastUpdateMillis;
    mSecondsTick = mMillisTick / 1000.0f;
    mLastUpdateMillis = mMillis;
}
