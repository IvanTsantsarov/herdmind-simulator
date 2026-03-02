#include "loradev.h"
#include "gateway/gateway.h"
#include "hardware/tools.h"
#include "../simtools.h"


#ifdef SIMULATION

#define SIMNODE_SENDING_DURATION 2000
#define FCtrl_ACK_bit (1 << 5)

uint32_t LoraDev::NODE_ADDR = 1000;



LoraDev::LoraDev(const QString &name,
                 Profile profile,
                 int updateInterval,
                 int sendInterval,
                 const QByteArray& devEUI,
                 const QByteArray& aSKey,
                 const QByteArray& nSKey
                  )
    : QObject(nullptr), mUpdateInterval(updateInterval), mSendInterval(sendInterval)
{
    mName = name;
    mProfile = profile;
    mDevEUI = devEUI.size() ? devEUI : QByteArray::fromHex( SimTools::genHex(EUI_BYTES_LEN) );
    mAppSKey = aSKey.size() ? aSKey : QByteArray::fromHex( SimTools::genAesKey() );
    mNwkSKey = nSKey.size() ? nSKey : QByteArray::fromHex( SimTools::genAesKey() );

    QTimer delayTimer;
    delayTimer.singleShot( Tools::rnd(0, sendInterval), this, &LoraDev::onTimerStart );
}

void LoraDev::setKeys(const QString &devEUI,
                      const QString &devAddr,
                      const QString &aSKey,
                      const QString &nSKey)
{
    mDevEUI = QByteArray::fromHex( devEUI.toLatin1() );
    setAddress( QByteArray::fromHex( devAddr.toLatin1()) );
    mAppSKey = QByteArray::fromHex( aSKey.toLatin1() );
    mNwkSKey = QByteArray::fromHex( nSKey.toLatin1() );
}

bool LoraDev::setFromJson(const QJsonObject &jobj)
{
    if( !jobj.contains("devEui") ||
        !jobj.contains("devAddr") ||
        !jobj.contains("appSKey")  ||
        !jobj.contains("nwkSKey")) {
        return false;
    }

    setKeys( jobj["devEui"].toString(),
             jobj["devAddr"].toString(),
             jobj["appSKey"].toString(),
             jobj["nwkSKey"].toString() );

    return true;
}


void LoraDev::updateSendingSimulation(int msec) {
    if( mSendingMsec <= 0 ) {
        return;
    }

    mSendingMsec -= msec;
}

QString LoraDev::jsonInfo(const QString& animalName)
{
    if( !animalName.length() ) {
        return QString( "{ \"devEui\":\"%1\","
                        "\"devAddr\":\"%2\","
                        "\"appSKey\":\"%3\","
                        "\"nwkSKey\":\"%4\" }" )
            .arg(mDevEUI.toHex())
            .arg(mDevAddr.toHex())
            .arg(mAppSKey.toHex())
            .arg(mNwkSKey.toHex());
    }

    return QString( "{ \"name\":\"%1\","
                   "\"devEui\":\"%2\","
                   "\"devAddr\":\"%3\","
                   "\"applicationId\":\"%4\","
                   "\"deviceProfileId\":\"%5\","
                   "\"appSKey\":\"%6\","
                   "\"nwkSKey\":\"%7\" }" )
        .arg(mName)
        .arg(mDevEUI.toHex())
        .arg(mDevAddr.toHex())
        .arg(gSimTools->appId())
        .arg(gSimTools->profileId(mProfile))
        .arg(mAppSKey.toHex())
        .arg(mNwkSKey.toHex());
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
#ifdef SIMULATION
    uplink( QByteArray(static_cast<char*>(package), size) );
#else
#endif
}


QByteArray LoraDev::cryptPayload(const QByteArray& payload,
                                 quint32 frameCounter,
                                 bool isDownlink,
                                 bool isMacCommand)
{
    QByteArray encrypted = payload;
    int blocks = (payload.size() + 15) / 16;

    for (int i = 0; i < blocks; i++) {

        QByteArray Ai(16, 0x00);

        Ai[0] = 0x01;         // encryption flags
        Ai[5] = isDownlink ? 0x01 : 0x00;         // Dir = uplink

        memcpy(Ai.data() + 6, mDevAddrRev.constData(), 4);


        // for sure on non x86 architecture
        Ai[10] = frameCounter & 0xFF;
        Ai[11] = (frameCounter >> 8) & 0xFF;
        Ai[12] = (frameCounter >> 16) & 0xFF;
        Ai[13] = (frameCounter >> 24) & 0xFF;

        Ai[15] = i + 1;

        QByteArray Si = SimTools::encryptAES( Ai, isMacCommand ? mNwkSKey : mAppSKey );

        for (int j = 0; j < 16; j++) {
            int index = i * 16 + j;
            if (index < encrypted.size())
                encrypted[index] ^= Si[j];
        }
    }

    return encrypted;
}


QByteArray LoraDev::calculateMIC(const QByteArray& msg, quint32 fCnt,
                                 bool isDownlink )
{
    QByteArray B0(16, 0x00);

    B0[0] = 0x49;
    B0[5] = isDownlink ? 0x01 : 0x00; // uplink

    memcpy(B0.data() + 6, mDevAddrRev.constData(), 4);


    B0[10] = fCnt & 0xFF;
    B0[11] = (fCnt >> 8) & 0xFF;
    B0[12] = (fCnt >> 16) & 0xFF;
    B0[13] = (fCnt >> 24) & 0xFF;

    B0[15] = static_cast<quint8>(msg.size());

    QByteArray cmacInput = B0 + msg;

    QByteArray fullCmac = SimTools::aesCmac( cmacInput, mNwkSKey );

    return fullCmac.left(4);
}

bool LoraDev::uplink(const QByteArray& data)
{
    QByteArray encrypted = cryptPayload( data, mFCntUp, false, false );

    QByteArray phy;

    phy.append((quint8)0x40);  // MHDR

    phy.append(mDevAddrRev);

    phy.append(mIsUplinkReceived ? (quint8)FCtrl_ACK_bit : (quint8)0x00);  // FCtrl

    quint16 fCnt16 = mFCntUp & 0xFFFF;
    phy.append(static_cast<char>(fCnt16 & 0xFF));
    phy.append(static_cast<char>((fCnt16 >> 8) & 0xFF));

    phy.append((quint8)LORA_FPORT);     // FPort
    phy.append(encrypted);

    QByteArray mic = calculateMIC(phy, mFCntUp, false);

    phy.append(mic);

    if( mGateway && mGateway->publish(phy) ) {
        mFCntUp ++;
        mIsUplinkReceived = false;
        return true;
    }

    return false;
}

void LoraDev::setAddress(const QByteArray &ba)
{
    mDevAddr = ba;
    mDevAddrRev.reserve(ba.size());
    for( auto i = 1; i <= ba.size(); i ++) {
        mDevAddrRev.append( ba[ba.size() - i] );
    }
}

void LoraDev::setGateway(Gateway *gw)
{
    mGateway = gw;
    connect(mGateway, &Gateway::downlinkReceived, this, &LoraDev::onDownlink );
}

void LoraDev::onDownlink(const QByteArray& phy)
{
    if( phy.size() < 12 ) {
        // Invalid phy
        return;
    }

    quint8 mhdr = phy[0];

    const QByteArray devAddr = phy.mid(1, 4);

    if( devAddr != mDevAddrRev ) {
        // Not for this device
        return;
    }

    const int micLen = 4;
    const int payloadLen = phy.size();

    quint8 fCtrl = phy[5];
    quint8 fOptsLen = fCtrl & 0x0F;

    // FHDR starts at 1:
    // DevAddr(4) + FCtrl(1) + FCnt(2) = 7 bytes
    // so FOpts starts at index 8
    const int fOptsStart = 8;
    const int fPortIndex = fOptsStart + fOptsLen;

    // If there is no FRMPayload, then packet ends right after FHDR + MIC.
    // That means fPortIndex points into MIC area, so there is no FPort.
    const bool hasFPortAndFrmPayload = (fPortIndex < payloadLen - micLen);

    if( !hasFPortAndFrmPayload ) {
        qDebug() << "No payload for device:" << mName << mDevAddr.toHex();
        return;
    }


    if( fCtrl & FCtrl_ACK_bit ) {
        mIsUplinkReceived = true;
    }

    quint16 fCnt16 =
        static_cast<quint8>(phy[6]) |
        (static_cast<quint8>(phy[7]) << 8);
    quint32 fCnt32 = fCnt16;

    quint8 fPort = phy[fPortIndex];

    QByteArray frmPayload =
        phy.mid(fPortIndex + 1,
                phy.size() - (fPortIndex + 1) - 4);


    bool isMacCommand = false;
    if( 0 == fPort ) {
        isMacCommand = true;
        // ignore MAC command
        // return;
    }


    QByteArray mic = phy.right(4);

    QByteArray calculated =
        calculateMIC(phy.left(phy.size() - 4), fCnt32, true);

    if (calculated != mic)
    {
        qWarning() << "Invalid downlink MIC for" << mName << mDevAddr.toHex();
        return;
    }


    // Decrypt using AppSKey
    QByteArray decrypted = cryptPayload( frmPayload, fCnt32, true, isMacCommand );


    if( isMacCommand ) {
        qInfo() << "#4e693a" << "MAC command for device" << mName << mDevAddr.toHex() << decrypted.toHex();
    }else {
        qInfo() << "#7517c2" << "Device received:" << mName << decrypted;
    }

    mFCntDown = fCnt32 + 1;

    onDownlinkDecrypted( isMacCommand ? decrypted.toHex() : decrypted );
}


void LoraDev::onDownlinkDecrypted(const QByteArray &content)
{

}


#else

#endif

