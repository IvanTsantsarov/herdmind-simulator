#ifndef COLLAR_H
#define COLLAR_H

#include <cmath>
#include <cstdint>

#include "../arduino.h"
#include "../defines.h"
#include "defines.h"
#include "../protocol.h"
#include "geometry.h"
#include "cli.h"

class Timer;
class GPS;
class Screen;
class Button;
class Led;
class CLI;
class Battery;
class Memory;
class Lights;
class Sound;
class Shock;
class Ble;

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

    class State {
        friend class Collar;

        public:
            enum Type {
                None = 0,
                Init,
                SetupBoot,
                SetupWake,
                Update,
                UpdateMaster,
                UpdateSlave,
                Sleep
            };

        private:
            Type mType = None;
            Type mTypePrev = None;
            int64_t mMsec = 0;
            int64_t mMsecElapsed = 0;
            int64_t mSecElapsed = 0;
            int64_t mMsecStarted = 0;
            int64_t mMsecDuration = 0;
            bool mIsNewSecond = false;
            uint8_t mTicks = 0;

        protected:
            void set(Type t, int64_t msecD = 0) {
                mSecElapsed = mMsecElapsed = 0;
                mIsNewSecond = false;
                mTicks = 0;
                mMsecDuration = msecD;

                mTypePrev = mType;
                mType = t;

                mMsecStarted = millis();
            }

            inline void setPrev() {
                set(mTypePrev);
            }

            void tick() {
                mMsec = millis();
                mMsecElapsed = mMsec - mMsecStarted;
                int64_t newSeconds = mMsecElapsed / 1000LL;
                mIsNewSecond = newSeconds > mSecElapsed;
                mSecElapsed = newSeconds;
                if( mTicks < 10) {
                    mTicks ++;
                }
            }

            void restart() {
                mMsecStarted = millis();
            }

        public:
            inline Type type(){ return mType; }
            inline bool is(const Type t){ return t == mType; }

            inline bool isInit(){ return Type::Init == mType; }
            inline bool isSleep(){ return Type::Sleep == mType; }

            const char* typeStr() {
                switch(mType) {
                case None: return "None";
                case Init: return "Init";
                case SetupBoot: return "SetupBoot";
                case SetupWake: return "SetupWake";
                case Update: return "Update";
                case UpdateMaster: return "UpdateMaster";
                case UpdateSlave: return "UpdateSlave";
                case Sleep: return "Sleep";
                }
            }

            inline int64_t durationMsec() {
                return mMsecDuration;
            }

            inline int64_t elapsedMsec(){ return mMsecElapsed; }
            inline int64_t elapsedSec(){ return mSecElapsed; }
            inline bool isFinished(){ return elapsedMsec() > mMsecDuration; }
            inline bool isNewSecond(){ return mIsNewSecond; }
            inline bool isElapsed(const int64_t msec){ return msec < mMsecElapsed; }
            inline bool isElapsedSec(const int64_t sec){ return sec < mSecElapsed; }
            inline bool isFirstTick(){ return 1 == mTicks; }
            inline int64_t startedMsec(){ return mMsecElapsed;}
            inline int64_t msec(){ return mMsec;}
    };

    int64_t mSendIntervalMsec = 30000;
    int64_t mSecWakeInterval = 30;

    State mState;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum ScreenMode : uint8_t {
        Product = 0,
        Info,
        Last,
        Bluetooth
    };

    ScreenMode mScreenMode = ScreenMode::Product;

    void incrementScreenMode() {
        mScreenMode = (ScreenMode)((uint8_t)mScreenMode + 1);
        if( mScreenMode >= ScreenMode::Last) {
            mScreenMode = ScreenMode::Product;
        }
    }



#ifdef ONPC
    friend class DialogCollarSim;
    Animal* mAnimal = nullptr;
    GeoPoint mInjectedPos;
    float mInjectedRSSI = 0;
    float mInjectedSNR = 0;
    int mInjectedBat = 0;
    bool mEnableRestore = false;
#else
    bool mEnableRestore = true;
#endif

    bool mIsStats = true;

    Timer* mTimer = nullptr;
    Screen* mScreen = nullptr;
    GPS* mGPS = nullptr;
    Button* mBtnMain = nullptr;
    Led* mLed = nullptr;
    CLI* mSerialCmd = nullptr;
    Battery* mBattery = nullptr;
    Memory* mMemory = nullptr;
    Lights* mLights = nullptr;
    Sound* mSound = nullptr;
    Shock* mShock = nullptr;
    Ble* mBle = nullptr;
    CLI* mBleCmd = nullptr;

    uint16_t mSequence = 0;

    bool mIsSignalGPS = false;
    GeoPoint readGPS();
    void printGPS();

    float mFenceDistanceSound1;
    float mFenceDistanceSound2;
    float mFenceDistanceSoundShock;
    bool mIsInsideFence = true;

    Protocol::Collar mPackage;

    void onSetupFence(uint8_t count, const GeoPoint &center, const uint8_t *offsetsPtr);

    void sendEvent(Protocol::Collar::Event event, uint32_t value);

    void onMainBtn(int64_t msecPress);

    void commonConstructor();
    void updateMaster();
    void updateCommunications();

    void updateScreen(bool isIncrement = false);
    void updateScreenProduct(bool isFlush = true);
    void updateScreenInfo(bool isFlush = true);


    enum struct UpdateInterval{
        Battery = 6000,
        CLI = 40,
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
    void inject(GeoPoint _pos, int _sat, float _rssi, float _snr, int _bat);
#else
    Collar();
#endif
    void sendStats();

    ~Collar();

    void setup();
    void update();
    void onSend();
    void onReceive(uint8_t* data, uint32_t size);
    inline Button* btnMain(){ return mBtnMain; }

    inline Screen *screen() { return mScreen; }
    inline Led* led() { return mLed; }

    void reboot();
    bool gotoBed();

#ifdef ONPC
    virtual float rssi(); // RSSI (Received Signal Strength Indicator) in dBm
    virtual float snr(); // SNR (Signal-to-Noise Ratio)
#endif

    char gpsStatus(); // y - has gps hardware, n - doesn't have gps harware, e - emulated gps
    GeoPoint gpsPos();
    inline GPS* gps(){ return mGPS; }
    String gpsStr();
    int satellites();

    int signalStrength(); // RSSI

    String batteryInfo();

    bool flash();
    bool restore();

    void serialRespond(const String &msg);
    void serialCmd(CLI::Cmd::Type t, const String &str = String());

    inline void setStats(bool is){ mIsStats = is; }
    inline bool toggleStats(){ mIsStats = !mIsStats; return mIsStats; }

    inline void setDbg(bool is){ gIsDbg = is; }
    inline bool toggleDbg(){ gIsDbg = !gIsDbg; return gIsDbg; }

    enum struct SignalType {
        Light = 0,
        Sound,
        Shock
    };

    void execSignal(SignalType t, int mode = 0);

    void setEpoch(uint64_t secs);
    uint64_t epoch();
    String dateTime();

    void setSendInterval(uint32_t msec){mSendIntervalMsec = msec;};
    uint32_t sendInterval(){ return mSendIntervalMsec; };

    void onBleConnect(String str);
    void onBleDisconnect(String str);

    String getMemInfo();
};


#ifndef ONPC
    extern Collar* gCollar;
#endif

#endif // COLLAR_H
