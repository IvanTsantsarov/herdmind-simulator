#include "netnode.h"
#include "hardware/tools.h"


#ifdef SIMULATION

#define SIMNODE_SENDING_DURATION 2000

uint32_t NetNode::NODE_ID = 1000;

NetNode::NetNode(int updateInterval, int sendInterval)
    : QObject(nullptr), mUpdateInterval(updateInterval), mSendInterval(sendInterval)
{
    QTimer delayTimer;
    delayTimer.singleShot( Tools::rnd(0, sendInterval), this, &NetNode::onTimerStart );
}

void NetNode::updateSendingSimulation(int msec) {
    if( mSendingMsec <= 0 ) {
        return;
    }

    mSendingMsec -= msec;
}


void NetNode::onTimerStart()
{
    connect( &mTimerUpdate, &QTimer::timeout, this, &NetNode::onTimerUpdate );
    mTimerUpdate.start(mUpdateInterval);
}

void NetNode::onTimerUpdate()
{
    onUpdate();

    mSendingDelay += mUpdateInterval;

    if( mSendingDelay >= mSendInterval ) {
        onSend();
        mSendingDelay = 0;
    }
}


void NetNode::sendPackage(void *package, int size)
{
    mSendingMsec = SIMNODE_SENDING_DURATION;
    mReadings ++;
}

#else

#endif
