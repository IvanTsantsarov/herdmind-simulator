#include "loradev.h"
#include "gateway/gateway.h"
#include "hardware/tools.h"
#include "../simtools.h"

namespace {
constexpr quint8 CID_LinkADRReq    = 0x03;
constexpr quint8 CID_LinkADRAns    = 0x03;
constexpr quint8 CID_NewChannelReq = 0x07;
constexpr quint8 CID_NewChannelAns = 0x07;

// Uplink FCtrl bits
constexpr quint8 FCtrl_ACK_bit       = (1 << 5);
constexpr quint8 FCtrl_FOptsLen_mask = 0x0F;

// Keep FOpts <= 15 bytes
constexpr int MaxFOptsLen = 15;
}

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
    QByteArray fOpts = mPendingMacAns.left(MaxFOptsLen);
    mPendingMacAns.remove(0, fOpts.size());

    QByteArray encrypted = cryptPayload(data, mFCntUp, false, false);

    QByteArray phy;
    phy.append(static_cast<char>(0x40));  // Unconfirmed Data Up
    phy.append(mDevAddrRev);

    quint8 fCtrl = 0x00;
    if (mIsUplinkReceived) {
        fCtrl |= FCtrl_ACK_bit;
    }
    fCtrl |= static_cast<quint8>(fOpts.size()) & FCtrl_FOptsLen_mask;
    phy.append(static_cast<char>(fCtrl));

    quint16 fCnt16 = mFCntUp & 0xFFFF;
    phy.append(static_cast<char>(fCnt16 & 0xFF));
    phy.append(static_cast<char>((fCnt16 >> 8) & 0xFF));

    // FOpts go here
    phy.append(fOpts);

    // Your app payload still goes on the normal FPort
    phy.append(static_cast<char>(fport()));
    phy.append(encrypted);

    QByteArray mic = calculateMIC(phy, mFCntUp, false);
    phy.append(mic);

    if (mGateway && mGateway->publish(phy)) {
        mFCntUp++;
        mIsUplinkReceived = false;
        return true;
    }

    // Put MAC answers back if publish failed
    mPendingMacAns = fOpts + mPendingMacAns;
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
    if (phy.size() < 12) {
        return;
    }

    const QByteArray devAddr = phy.mid(1, 4);
    if (devAddr != mDevAddrRev) {
        return;
    }

    quint8 fCtrl = static_cast<quint8>(phy[5]);
    quint8 fOptsLen = fCtrl & 0x0F;

    quint16 fCnt16 =
        static_cast<quint8>(phy[6]) |
        (static_cast<quint8>(phy[7]) << 8);
    quint32 fCnt32 = fCnt16;

    QByteArray mic = phy.right(4);
    QByteArray calculated = calculateMIC(phy.left(phy.size() - 4), fCnt32, true);
    if (calculated != mic) {
        qWarning() << "Invalid downlink MIC for" << mName << mDevAddr.toHex();
        return;
    }

    if (fCtrl & FCtrl_ACK_bit) {
        mIsUplinkReceived = true;
    }

    const int fOptsStart = 8;
    QByteArray fOpts = phy.mid(fOptsStart, fOptsLen);

    // Parse MAC commands in FOpts first
    if (!fOpts.isEmpty()) {
        qInfo() << "MAC commands in FOpts for" << mName << mDevAddr.toHex()
        << fOpts.toHex();
        onDownlinkDecrypted(fOpts);
    }

    const int micLen = 4;
    const int fPortIndex = fOptsStart + fOptsLen;
    const bool hasFPort = (fPortIndex < phy.size() - micLen);

    if (!hasFPort) {
        mFCntDown = fCnt32 + 1;
        return;
    }

    quint8 fPort = static_cast<quint8>(phy[fPortIndex]);
    QByteArray frmPayload =
        phy.mid(fPortIndex + 1,
                phy.size() - (fPortIndex + 1) - micLen);

    if (frmPayload.isEmpty()) {
        mFCntDown = fCnt32 + 1;
        return;
    }

    bool isMacCommand = (fPort == 0);
    QByteArray decrypted = cryptPayload(frmPayload, fCnt32, true, isMacCommand);

    if (isMacCommand) {
        qInfo() << "MAC commands in FRMPayload for" << mName << mDevAddr.toHex()
        << decrypted.toHex();
        onDownlinkDecrypted(decrypted);
    } else {
        qInfo() << "Device received:" << mName << decrypted;
        emit messageReceived(mDevAddr, decrypted);
    }

    mFCntDown = fCnt32 + 1;
}

void LoraDev::onDownlinkDecrypted(const QByteArray &raw)
{
    int i = 0;
    while (i < raw.size()) {
        const quint8 cid = static_cast<quint8>(raw[i]);

        switch (cid) {
        case CID_LinkADRReq: {
            if (i + 5 > raw.size()) {
                qWarning() << "Truncated LinkADRReq for" << mName << mDevAddr.toHex();
                return;
            }

            quint8 status = 0x07; // Channel mask ACK + Data rate ACK + TX power ACK

            if (mPendingMacAns.size() + 2 <= MaxFOptsLen) {
                mPendingMacAns.append(static_cast<char>(CID_LinkADRAns));
                mPendingMacAns.append(static_cast<char>(status));
            }

            i += 5;
            break;
        }

        case CID_NewChannelReq: {
            if (i + 6 > raw.size()) {
                qWarning() << "Truncated NewChannelReq for" << mName << mDevAddr.toHex();
                return;
            }

            quint8 status = 0x03; // Data-rate range OK + Channel frequency OK

            if (mPendingMacAns.size() + 2 <= MaxFOptsLen) {
                mPendingMacAns.append(static_cast<char>(CID_NewChannelAns));
                mPendingMacAns.append(static_cast<char>(status));
            }

            i += 6;
            break;
        }

        case 0x05: { // RXParamSetupReq
            if (i + 5 > raw.size()) {
                qWarning() << "Truncated RXParamSetupReq for" << mName << mDevAddr.toHex();
                return;
            }

            // status bits:
            // bit0 = Channel ACK
            // bit1 = RX2 data-rate ACK
            // bit2 = RX1 DR offset ACK
            quint8 status = 0x07; // accept all for now

            if (mPendingMacAns.size() + 2 <= MaxFOptsLen) {
                mPendingMacAns.append(char(0x05));
                mPendingMacAns.append(char(status));
            }

            i += 5;
            break;
        }

        case 0x08: { // RXTimingSetupReq
            if (i + 2 > raw.size()) {
                qWarning() << "Truncated RXTimingSetupReq for" << mName << mDevAddr.toHex();
                return;
            }

            if (mPendingMacAns.size() + 1 <= MaxFOptsLen) {
                mPendingMacAns.append(char(0x08));
            }

            i += 2;
            break;
        }

        case 0x11: { // PingSlotChannelReq, Class B only
            if (i + 5 > raw.size()) {
                qWarning() << "Truncated PingSlotChannelReq for" << mName << mDevAddr.toHex();
                return;
            }

            // If you do not implement Class B, better disable Class B in the device profile.
            // For testing, you could still ACK it, but that would be pretending to support Class B.
            quint8 status = 0x03; // frequency + data-rate OK

            if (mPendingMacAns.size() + 2 <= MaxFOptsLen) {
                mPendingMacAns.append(char(0x11));
                mPendingMacAns.append(char(status));
            }

            i += 5;
            break;
        }

        default:
            qWarning() << "Unsupported MAC CID for" << mName
                       << mDevAddr.toHex()
                       << "cid=0x" << QByteArray::number(cid, 16);
            return;
        }
    }
}
#else

#endif

