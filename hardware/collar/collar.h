#ifndef COLLAR_H
#define COLLAR_H

#include <cmath>
#include <cstdint>

#include "../arduino.h"
#include "../defines.h"
#include "defines.h"
#include "../protocol.h"
#include "geometry.h"
#include "serialcmd.h"

#define COLLAR_MAX_GPS_POINTS 500

class Timer;
class GPS;
class Screen;
class Button;
class Led;
class SerialCmd;
class Battery;
class Memory;
class Lights;
class Sound;
class Shock;

#ifdef ONPC
    #include <QPointF>
    #include <QLine>
#endif

#include "loradev.h"

#ifdef ONPC
class Collar : public LoraDevSim
#else
class Collar : public LoraDevHW
#endif

{
    friend void gMainButtonInterrupt();

    enum struct Stage {
        None = 0,
        Setup = 1,
        Init = 2,
        Operate = 3,
        Sleep
    };

    int64_t mSecSleep = 0;
    int64_t mSendIntervalMsec = 15000;

    Stage mStage = Stage::None;
    String mAnimalName;
    bool mIsMale = false;

#ifdef ONPC
    friend class DialogCollarSim;
    Animal* mAnimal = nullptr;
    GeoPoint mInjectedPos;
    int mInjectedRSSI = 0;
    int mInjectedSNR = 0;
    int mInjectedBat = 0;
    bool mEnableRestore = false;
#else
    int64_t mStatsMsec = 0;
    bool mEnableRestore = true;
#endif

    bool mIsStats = true;

    Timer* mTimer = nullptr;
    Screen* mScreen = nullptr;
    GPS* mGPS = nullptr;
    Button* mBtnMain = nullptr;
    Led* mLed = nullptr;
    SerialCmd* mSerialCmd = nullptr;
    Battery* mBattery = nullptr;
    Memory* mMemory = nullptr;
    Lights* mLights = nullptr;
    Sound* mSound = nullptr;
    Shock* mShock = nullptr;

    uint16_t mSequence = 0;
    int64_t mSetupMillis = 0;
    int64_t mAwakeningMillisScreen = 0;


    bool mIsSignalGPS = false;
    GeoPoint readGPS();
    void printGPS();
    GeoPoint mLastGeoPos;
    Point mLastPoint;

    float mFenceDistanceSound1;
    float mFenceDistanceSound2;
    float mFenceDistanceSoundShock;
    bool mIsInsideFence = true;

    GeoPoint mGeoCenter;
    int mFencePointsCount = 0;
    GeoPoint mFenceGeoPoints[VIRTUAL_FENCE_MAX_POINTS];
    Point mFencePoints[VIRTUAL_FENCE_MAX_POINTS];
    Border mFenceBorders[VIRTUAL_FENCE_MAX_POINTS];

    Border* mFenceClosestBorder = nullptr;
    Point mFenceClosestPoint;
    double mFenceClosestDistSq;
    double mFenceDistance;
    bool mFenceIsGoingAway = false;

    int mTrajectoryPointsCount = 0;
    GeoPoint mTrajectoryPoints[COLLAR_MAX_GPS_POINTS];

    void testFence();
    Protocol::Collar mPackage;

    void onSetupFence(uint8_t count,
                      const GeoPoint &center,
                      const uint8_t *offsetsPtr);

    void sendEvent(Protocol::Collar::Event event, uint32_t value);

    void onMainBtn();
    void updateTrajectory(const GeoPoint &geoPt);
    void commonConstructor();
    void updateScreenLoading(bool isFlush = true);
    void updateScreenNormal(bool isFlush = true);
    void updateSerial();

    enum struct UpdateInterval{
        Battery = 6000,
        SerialCmd = 40,
        GPS = 1000
    };

public:


#ifdef ONPC
    Collar( Animal* animal, bool isSimulation,
           const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray(),
           const QByteArray& nwkKey = QByteArray() );

    Collar(QString animalName, bool isMale,
           const QByteArray& devEUI,
           const QByteArray& appKey,
           const QByteArray& nwkKey ,
           const QByteArray& addrHex);


    Protocol::Collar getPackageOut();
    QList<Protocol::Collar> getBoluses();
    QLine fenceClosestBorder();
    QPointF fenceClosestPoint();
    inline const Animal* animal() const { return mAnimal; }
    void sendToSerial(const char* str);
    QByteArray readFromSerial();
    void inject(GeoPoint _pos, int _sat, int _rssi, int _snr, int _bat);
#else
    Collar();
    void sendStats();
#endif

    ~Collar();

    void setAnimalName(const String& name);
    inline void setIsMale(bool is){ mIsMale = is; }

    inline String& animalName() { return mAnimalName; }
    inline bool isMale(){ return mIsMale; }
    void onSetup();
    void onUpdate();
    void onSend();
    void onReceive(uint8_t* data, uint32_t size);
    inline Button* btnMain(){ return mBtnMain; }

    inline bool isFence(){ return mFencePointsCount > 0; }
    inline bool isInsideFence(){ return mIsInsideFence; }
    inline bool isGoingAwayFromFence(){ return mFenceIsGoingAway; }
    inline double fanceDistance(){ return mFenceDistance; }
    inline bool hasClosestFenceBorder(){ return nullptr != mFenceClosestBorder ; }
    inline Screen *screen() { return mScreen; }
    inline Led* led() { return mLed; }

    void reboot();
    bool gotoBed();

#ifdef ONPC
    virtual int rssi(); // RSSI (Received Signal Strength Indicator) in dBm
    virtual int snr(); // SNR (Signal-to-Noise Ratio)
#endif

    char gpsStatus(); // y - has gps hardware, n - doesn't have gps harware, e - emulated gps
    GeoPoint gps();
    String gpsStr();
    int satellites();

    int signalStrength(); // RSSI

    String batteryInfo();

    bool flash();
    bool restore();

    void serialRespond(const String &msg);
    void serialCmd(SerialCmd::Cmd::Type t, const String &str = String());

    inline void setStats(bool is){ mIsStats = is; }
    inline bool toggleStats(){ mIsStats = !mIsStats; return mIsStats; }

    inline void setDbg(bool is){ gIsDbg = is; }
    inline bool toggleDbg(){ gIsDbg = !gIsDbg; return gIsDbg; }

    enum struct SignalType {
        Light = 0,
        Sound,
        Shock
    };

    void execSignal(SignalType t);

    void setEpoch(uint64_t secs);
    uint64_t epoch();
    String dateTime();

    void setSendInterval(uint32_t msec){mSendIntervalMsec = msec;};
    uint32_t sendInterval(){ return mSendIntervalMsec; };
};


#ifndef ONPC
    extern Collar* gCollar;
#endif

#endif // COLLAR_H
