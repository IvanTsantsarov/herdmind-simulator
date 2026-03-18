#pragma once

#ifdef SIMULATION

#include <QJsonObject>
#include <QObject>
#include <QTimer>

class Animal;
class Gateway;

#define EUI_BYTES_LEN 8
#define LORA_FPORT_START 9

class LoraDev : public QObject
{
    Q_OBJECT

public:
    enum struct Profile {
        None = 0,
        Bolus = 1,
        Collar = 2
    };
private:

    QString mName;
    Profile mProfile = LoraDev::Profile::None;

    static uint32_t NODE_ADDR;

    QByteArray mDevEUI; // build-in unique 8 bytes like MAC address (ex:0x1234567890AAAAAA)
    QByteArray mNwkSKey;
    QByteArray mAppSKey;

    // 4 bytes - dynamic address, obtained from chirpstack after accepting connection
    QByteArray mDevAddr, mDevAddrRev;

    uint32_t mFCntUp = 0;
    uint32_t mFCntDown = 0;

    uint16_t mDevNonce = 1;

    uint32_t mUpdateInterval = 0;
    uint32_t mSendInterval = 0;
    uint32_t mSendingDelay = 0;

    QByteArray cryptPayload(const QByteArray& payload, quint32 frameCounter,
                            bool isDownlink,
                            bool isMacCommand );

    // Message Integrity Code
    QByteArray calculateMIC(const QByteArray& data, quint32 fCnt, bool isDownlink);

    QByteArray buildPHYPayload(QByteArray frmPayload,
                               quint32 devAddr);

    void onDownlinkDecrypted(const QByteArray &content);

    QTimer mTimerUpdate;
    int mSendingMsec = 0;
    int mReadings = 0;

    bool mIsUplinkReceived = false;

    Gateway* mGateway = nullptr;

    QByteArray mPendingMacAns;   // bytes to send in next uplink as FOpts

protected:

    // send to the chirpstack
    bool uplink(const QByteArray& data);


public:
    inline QByteArray eui(){ return mDevEUI; };
    inline QByteArray addr(){ return mDevAddr; };
    inline QString name(){ return mName; }
    inline Profile profile(){ return mProfile; }
    inline QByteArray appSKey(){ return mAppSKey; }
    inline QByteArray nwkSKey(){ return mNwkSKey; }
    inline bool isCollar(){ return Profile::Collar == mProfile; }
    inline bool isBolus(){ return Profile::Bolus == mProfile; }
    inline bool isValid(){ return Profile::None != mProfile; }
    void setAddress(const QByteArray& ba);;
    void setGateway(Gateway* gw);

    inline uint8_t fport(){ return (quint8)LORA_FPORT_START + (quint8)mProfile; }
    /// inline uint8_t fport(){ return 1; }

    LoraDev( const QString& name,
            Profile profile,
            int updateInterval, int sendInterval,
            const QByteArray &devEUI = QByteArray(),
            const QByteArray& aSKey = QByteArray(),
            const QByteArray& nSKey = QByteArray() );

    void setKeys(const QString &devEUI,
                 const QString &devAddr,
                 const QString &aSKey , const QString &nSKey);

    bool setFromJson(const QJsonObject &jobj );


    // inline QByteArray key(){ return mAppKey; };


    void sendPackage(void* package, int size);

    void updateSendingSimulation(int msec);

    bool isSending() { return mSendingMsec > 0; }

    virtual void onUpdate() = 0; // On regular sensors update
    virtual void onSend() = 0; // On timeout for sending

    // if animal name is specified - returns full info for chirpstack device registration
    QString jsonInfo(const QString &animalName = QString());
signals:
    void messageReceived(const QByteArray& addr, const QByteArray& msg);

private slots:
    void onTimerStart();
    void onTimerUpdate();
    void onDownlink(const QByteArray& phy);


};

#else
class LoraDev
{

};
#endif
