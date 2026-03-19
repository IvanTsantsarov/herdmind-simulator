#ifndef SIMTIMER_H
#define SIMTIMER_H

#include <QObject>

class QElapsedTimer;

class SimTimer : public QObject
{
    Q_OBJECT


    quint64 mLastUpdateMillis = 0;
    quint64 mMillisTick = 0;
    double mSecondsTick = 0;

    quint64 mMillis = 0,
        mSeconds = 0,
        mMinutes = 0,
        mHours = 0,
        mDays = 0;

    QElapsedTimer* mElapsedTimer = nullptr;


public:
    explicit SimTimer(QObject *parent = nullptr);
    ~SimTimer();
    inline quint64 millis(){ return mMillis; }
    inline quint64 seconds(){ return mSeconds; }
    inline quint64 minutes(){ return mMinutes; }
    inline quint64 hours(){ return mHours; }
    inline quint64 days(){ return mDays; }
    inline quint64 tickMillis(){ return mMillisTick; }
    inline double tickSeconds(){ return mSecondsTick; }

    void update();

signals:
};

extern SimTimer* gSimTimer;

#endif // SIMTIMER_H
