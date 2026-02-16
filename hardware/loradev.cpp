#include "loradev.h"
#include "hardware/tools.h"
#include "../simtools.h"
#include "defines.h"


#ifdef SIMULATION

#define SIMNODE_SENDING_DURATION 2000

uint32_t LoraDev::NODE_ADDR = 1000;


LoraDev::LoraDev(const QString &name,
                 Profile profile,
                 int updateInterval,
                 int sendInterval,
                 const QByteArray& devEUI,
                 const QByteArray& appKey )
    : QObject(nullptr), mUpdateInterval(updateInterval), mSendInterval(sendInterval)
{
    mName = name;
    mProfile = profile;
    mDevAddr = ++ NODE_ADDR;
    mDevEUI = devEUI.size() ? devEUI : QByteArray::fromHex( SimTools::genHex(8) );
    mAppKey = appKey.size() ? appKey : QByteArray::fromHex( SimTools::genAesKey() );

    // mDevEUI = devEUI.size() ? devEUI : SimTools::genHex(8);
    // mAppSKey = appKey.size() ? appKey : SimTools::genAesKey();

    QTimer delayTimer;
    delayTimer.singleShot( Tools::rnd(0, sendInterval), this, &LoraDev::onTimerStart );
}

void LoraDev::updateSendingSimulation(int msec) {
    if( mSendingMsec <= 0 ) {
        return;
    }

    mSendingMsec -= msec;
}

QString LoraDev::jsonInfo()
{
    return QString( "{ \"name\":\"%1\","
                    "\"devEui\":\"%2\","
                    "\"applicationId\":\"%3\","
                    "\"deviceProfileId\":\"%4\","
                    "\"applicationKey\":\"%5\" }" )
        .arg(mName)
        .arg(mDevEUI.toHex())
        .arg(APP_ID)
        .arg((int)mProfile)
        .arg(mAppKey.toHex());
}


void LoraDev::onTimerStart()
{
    connect( &mTimerUpdate, &QTimer::timeout, this, &LoraDev::onTimerUpdate );
    mTimerUpdate.start(mUpdateInterval);
}

void LoraDev::onTimerUpdate()
{
    onUpdate();

    mSendingDelay += mUpdateInterval;

    if( mSendingDelay >= mSendInterval ) {
        onSend();
        mSendingDelay = 0;
    }
}


void LoraDev::sendPackage(void *package, int size)
{
    mSendingMsec = SIMNODE_SENDING_DURATION;
    mReadings ++;
}


QByteArray LoraDev::encryptFRMPayload(const QByteArray& payload )
{
    QByteArray encrypted = payload;
    int blocks = (payload.size() + 15) / 16;

    for (int i = 0; i < blocks; i++) {

        QByteArray Ai(16, 0x00);

        Ai[0] = 0x01;         // encryption flags
        Ai[5] = 0x00;         // Dir = uplink

        memcpy(Ai.data() + 6, &mDevAddr, 4);
        memcpy(Ai.data() + 10, &mFCnt, 4);

        Ai[15] = i + 1;

        QByteArray Si = SimTools::encryptAES( Ai, mAppSKey );

        for (int j = 0; j < 16; j++) {
            int index = i * 16 + j;
            if (index < encrypted.size())
                encrypted[index] ^= Si[j];
        }
    }

    return encrypted;
}


QByteArray LoraDev::calculateMIC( const QByteArray& msg )
{
    QByteArray B0(16, 0x00);

    B0[0] = 0x49;
    B0[5] = 0x00; // uplink

    memcpy(B0.data() + 6, &mDevAddr, 4);
    memcpy(B0.data() + 10, &mFCnt, 4);

    B0[15] = msg.size();

    QByteArray cmacInput = B0 + msg;

    QByteArray fullCmac = SimTools::aesCmac( cmacInput, mNwkSKey );

    return fullCmac.left(4);
}


bool LoraDev::sendToChirpstack(const QByteArray& data)
{
    QByteArray encrypted = encryptFRMPayload( data );

    QByteArray phy;

    phy.append((quint8)0x40);  // MHDR
    phy.append(reinterpret_cast<char*>(&mDevAddr), 4);
    phy.append((quint8)0x00);  // FCtrl

    quint16 fCnt16 = mFCnt & 0xFFFF;
    phy.append(reinterpret_cast<char*>(&fCnt16), 2);

    phy.append((quint8)1);     // FPort
    phy.append(encrypted);

    QByteArray mic = calculateMIC(phy);

    phy.append(mic);

    if( gSimTools->sendToChirpStack(phy) ) {
        mFCnt ++;
        return true;
    }

    return false;
}


#else

#endif
