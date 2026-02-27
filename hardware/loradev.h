#pragma once

#ifdef SIMULATION

#include <QJsonObject>
#include <QObject>
#include <QTimer>

class Animal;
class Gateway;

#define EUI_BYTES_LEN 8
#define LORA_FPORT 1

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
    QByteArray mAppKey; // application id is from chirpstack after creating new application

    // 4 bytes - dynamic address, obtained from chirpstack after accepting connection
    QByteArray mDevAddr, mDevAddrRev;

    uint32_t mFCnt = 0;

    uint16_t mDevNonce = 1;

    uint32_t mUpdateInterval = 0;
    uint32_t mSendInterval = 0;
    uint32_t mSendingDelay = 0;

    QByteArray cryptPayload(const QByteArray& payload,
                            quint32 frameCounter,bool isDownlink);

    // Message Integrity Code
    QByteArray calculateMIC( const QByteArray& data );

    QByteArray buildPHYPayload(QByteArray frmPayload,
                               quint32 devAddr);

    void onDownlinkDecrypted(const QByteArray &content);

    QTimer mTimerUpdate;
    int mSendingMsec = 0;
    int mReadings = 0;

    Gateway* mGateway = nullptr;
protected:

    bool sendSimulate(const QByteArray& data);


public:
    inline QByteArray eui(){ return mDevEUI; };
    inline QByteArray addr(){ return mDevAddr; };
    inline QString name(){ return mName; }
    inline Profile profile(){ return mProfile; }
    inline QByteArray appKey(){ return mAppKey; }
    inline bool isCollar(){ return Profile::Collar == mProfile; }
    inline bool isBolus(){ return Profile::Bolus == mProfile; }
    inline bool isValid(){ return Profile::None != mProfile; }
    void setAddress(const QByteArray& ba);;
    void setGateway(Gateway* gw);

    LoraDev( const QString& name,
            Profile profile,
            int updateInterval, int sendInterval,
            const QByteArray &devEUI = QByteArray(),
            const QByteArray& appKey = QByteArray() );

    void setKeys(const QString &devEUI,
                 const QString &devAddr,
                 const QString &appKey );

    bool setFromJson(const QJsonObject &jobj );


    // inline QByteArray key(){ return mAppKey; };


    void sendPackage(void* package, int size);

    void updateSendingSimulation(int msec);

    bool isSending() { return mSendingMsec > 0; }

    virtual void onUpdate() = 0; // On regular sensors update
    virtual void onSend() = 0; // On timeout for sending

    // if animal name is specified - returns full info for chirpstack device registration
    QString jsonInfo(const QString &animalName = QString());

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
