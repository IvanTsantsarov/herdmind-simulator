#pragma once

#ifdef SIMULATION

#include <QJsonObject>
#include <QObject>
#include <QTimer>

class Animal;
class SimTools;

#define EUI_BYTES_LEN 8

class LoraDev : public QObject
{
    Q_OBJECT

public:
    enum struct Profile {
        None = 0,
        Bolus = 1,
        Collar = 2,
        Gateway = 3
    };
private:

    QString mName;
    Profile mProfile = LoraDev::Profile::None;

    static uint32_t NODE_ADDR;

    QByteArray mDevEUI; // build-in unique 8 bytes like MAC address (ex:0x1234567890AAAAAA)
    QByteArray mJoinEUI; // build-in unique 8 bytes like MAC address for connecting only
    QByteArray mAppKey; // application id is from chirpstack after creating new application

    uint32_t mDevAddr = 0; // 4 bytes - dynamic address, obtained from chirpstack after accepting connection

    QByteArray mAppSKey;
    QByteArray mNwkSKey;

    uint32_t mFCnt = 0;

    QByteArray mDevNonce; // used only for joining


    uint32_t mUpdateInterval = 0;
    uint32_t mSendInterval = 0;
    uint32_t mSendingDelay = 0;

    QByteArray encryptFRMPayload(const QByteArray& payload);

    // Message Integrity Code
    QByteArray calculateMIC( const QByteArray& data );

    QByteArray buildPHYPayload(QByteArray frmPayload,
                               quint32 devAddr);

    QByteArray buildJoinRequest();

    QTimer mTimerUpdate;
    int mSendingMsec = 0;
    int mReadings = 0;

    SimTools* mSimTools = nullptr;
protected:

    bool sendToChirpstack(const QByteArray& data);


public:

    LoraDev( const QString& name,
            Profile profile,
            int updateInterval, int sendInterval,
            const QByteArray &devEUI = QByteArray(),
            const QByteArray &joinEUI = QByteArray(),
            const QByteArray& appKey = QByteArray() );

    void setKeys(const QString &devEUI,
                 const QString &joinEUI,
                 const QString &appKey );

    bool setFromJson(const QJsonObject &jobj );


    inline QByteArray eui(){ return mDevEUI; };
    inline uint32_t addr(){ return mDevAddr; };
    // inline QByteArray key(){ return mAppKey; };


    void sendPackage(void* package, int size);

    void updateSendingSimulation(int msec);

    bool isSendingSimulation() { return mSendingMsec > 0; }

    virtual void onUpdate() = 0; // On regular sensors update
    virtual void onSend() = 0; // On timeout for sending

    // if animal name is specified - returns full info for chirpstack device registration
    QString jsonInfo(const QString &animalName = QString());

    bool processJoinAccept(const QByteArray& phyPayload);

private slots:
    void onTimerStart();
    void onTimerUpdate();
private:
};

#else
class LoraDev
{

};
#endif
