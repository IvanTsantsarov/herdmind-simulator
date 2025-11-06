#pragma once

#include <QObject>
#include <QTimer>

class Animal;


#ifdef SIMULATION
class NetNode : public QObject
{
    Q_OBJECT

    uint32_t mUpdateInterval = 0;
    uint32_t mSendInterval = 0;
    uint32_t mSendingDelay = 0;

protected:

    static uint32_t NODE_ID;
    uint32_t mId = NODE_ID++;


private:
    QTimer mTimerUpdate;
    int mSendingMsec = 0;
    int mReadings = 0;

public:

    inline uint32_t id(){ return mId; };

    NetNode(int updateInterval, int sendInterval);

    void sendPackage(void* package, int size);

    void updateSendingSimulation(int msec);

    bool isSendingSimulation() { return mSendingMsec > 0; }

    virtual void onUpdate() = 0; // On regular sensors update
    virtual void onSend() = 0; // On timeout for sending

private slots:
    void onTimerStart();
    void onTimerUpdate();

};

#else
class NetNode
{

};
#endif
