#ifndef COLLAR_H
#define COLLAR_H

#include <cmath>
#include <cstdint>

#include "../arduino.h"
#include "../defines.h"
#include "../protocol.h"
#include "geometry.h"
#include "serialcmd.h"

#define COLLAR_MAX_GPS_POINTS 500

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
    int64_t mDbgMsec = 0;
    bool mEnableRestore = true;
#endif

    bool mIsDbgInfo = true;

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
    const Animal* animal() const;
    void sendToSerial(const char* str);
    QByteArray readFromSerial();
    void inject(GeoPoint _pos, int _sat, int _rssi, int _snr, int _bat);
#else
    Collar();
    void sendDbg();
#endif


    ~Collar();

    void setAnimalName(const String& name);
    void setIsMale(bool is);

    String& animalName();
    bool isMale();
    void onSetup();
    void onUpdate();
    void onSend();
    void onReceive(uint8_t* data, uint32_t size);
    Button* btnMain();

    bool isFence();
    bool isInsideFence();
    bool isGoingAwayFromFence();
    double fanceDistance();
    bool hasClosestFenceBorder();
    Screen *screen();
    Led* led();

    void reboot();
    void sleep();

    int rssi(); // RSSI (Received Signal Strength Indicator) in dBm

    // SNR (Signal-to-Noise Ratio)
    int snr();

    GeoPoint gps();
    String gpsStr();
    int satellites();

    int signalStrength(); // RSSI

    String batteryInfo();

    bool flash();
    bool restore();

    void setDebugInfo(bool is);;
    bool toggleDebugInfo();;
    void serialRespond(const String &msg);
    void serialCmd(SerialCmd::Cmd::Type t, const String &str = String());
};

inline void Collar::setIsMale(bool is){ mIsMale = is; }

inline String &Collar::animalName() { return mAnimalName; }

inline bool Collar::isMale(){ return mIsMale; }

inline Button *Collar::btnMain(){ return mBtnMain; }

inline bool Collar::isFence(){ return mFencePointsCount > 0; }

inline bool Collar::isInsideFence(){ return mIsInsideFence; }

inline bool Collar::isGoingAwayFromFence(){ return mFenceIsGoingAway; }

inline double Collar::fanceDistance(){ return mFenceDistance; }

inline bool Collar::hasClosestFenceBorder(){ return nullptr != mFenceClosestBorder ; }

inline Screen *Collar::screen() { return mScreen; }

inline Led *Collar::led() { return mLed; }

inline void Collar::setDebugInfo(bool is){ mIsDbgInfo = is; }

inline bool Collar::toggleDebugInfo(){ mIsDbgInfo = !mIsDbgInfo; return mIsDbgInfo; }

#ifndef ONPC
    extern Collar* gCollar;
#endif

#endif // COLLAR_H
