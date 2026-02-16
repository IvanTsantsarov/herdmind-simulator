#pragma once

#ifdef SIMULATION

#include <QObject>
#include <QTimer>

class Animal;
class SimTools;

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

    QByteArray mDevEUI; // build-in 8 bytes like MAC address
    QByteArray mAppKey; // build-in 16 bytes aes key

    uint32_t mDevAddr = 0; // 4 bytes - dynamic address

    QByteArray mAppSKey;
    QByteArray mNwkSKey;

    uint32_t mFCnt = 0;

    uint32_t mUpdateInterval = 0;
    uint32_t mSendInterval = 0;
    uint32_t mSendingDelay = 0;

    QByteArray encryptFRMPayload(const QByteArray& payload);

    // Message Integrity Code
    QByteArray calculateMIC( const QByteArray& data );

    QByteArray buildPHYPayload(QByteArray frmPayload,
                               quint32 devAddr);

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
            const QByteArray& appKey = QByteArray() );

    inline QByteArray eui(){ return mDevEUI; };
    inline uint32_t addr(){ return mDevAddr; };
    // inline QByteArray key(){ return mAppKey; };


    void sendPackage(void* package, int size);

    void updateSendingSimulation(int msec);

    bool isSendingSimulation() { return mSendingMsec > 0; }

    virtual void onUpdate() = 0; // On regular sensors update
    virtual void onSend() = 0; // On timeout for sending

    QString jsonInfo();

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
