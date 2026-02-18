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
                 const QByteArray& devEUI, const QByteArray &joinEUI,
                 const QByteArray& appKey )
    : QObject(nullptr), mUpdateInterval(updateInterval), mSendInterval(sendInterval)
{
    mName = name;
    mProfile = profile;
    mDevEUI = devEUI.size() ? devEUI : QByteArray::fromHex( SimTools::genHex(EUI_BYTES_LEN) );
    mJoinEUI = joinEUI.size() ? joinEUI : QByteArray::fromHex( SimTools::genHex(EUI_BYTES_LEN) );
    mAppKey = appKey.size() ? appKey : QByteArray::fromHex( SimTools::genAesKey() );

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
                    "\"joinEui\":\"%3\","
                    "\"applicationId\":\"%4\","
                    "\"deviceProfileId\":\"%5\","
                    "\"applicationKey\":\"%6\" }" )
        .arg(mName)
        .arg(mDevEUI.toHex())
        .arg(gSimTools->joinEUI().toHex())
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


QByteArray LoraDev::buildJoinRequest()
{
    QByteArray payload;

    quint8 mhdr = 0x00; // JoinRequest
    payload.append(mhdr);

    payload.append(gSimTools->joinEUI());
    payload.append(mDevEUI);

    mDevNonce = QByteArray(2, 0);
    quint16 nonce = SimTools::gen(0, 0xFFFF);
    mDevNonce[0] = nonce & 0xFF;
    mDevNonce[1] = (nonce >> 8) & 0xFF;

    payload.append(mDevNonce);

    QByteArray mic = calculateMIC(payload);
    payload.append(mic);

    return payload;
}

bool LoraDev::processJoinAccept(const QByteArray& phyPayload)
{
    QByteArray encrypted = phyPayload.mid(1); // remove MHDR

    // Encrypt used for decrypt (LoRaWAN spec rule)
    QByteArray decrypted = SimTools::encryptAES(encrypted, mAppKey);

    QByteArray joinNonce = decrypted.mid(0, 3);
    QByteArray netId     = decrypted.mid(3, 3);
    QByteArray devAddr   = decrypted.mid(6, 4);

    mDevAddr =
        (quint8)devAddr[0] |
        ((quint8)devAddr[1] << 8) |
        ((quint8)devAddr[2] << 16) |
        ((quint8)devAddr[3] << 24);

    QByteArray block(16, 0);

    block[0] = 0x02;
    memcpy(block.data() + 1, joinNonce.data(), 3);
    memcpy(block.data() + 4, netId.data(), 3);
    memcpy(block.data() + 7, mDevNonce.data(), 2);

    mAppSKey = SimTools::encryptAES(block, mAppKey);

    block[0] = 0x01;
    mNwkSKey = SimTools::encryptAES(block, mAppKey);

    return true;
}

#else

#endif
