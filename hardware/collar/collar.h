#ifndef COLLAR_H
#define COLLAR_H

#include <cmath>
#include <cstdint>

#include "geometry.h"

#include "../defines.h"
#include "../protocol.h"

#include "../arduino.h"

// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 100

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 100

#define COLLAR_MAX_GPS_POINTS 500

class GPS;
class Screen;
class Button;
class Led;
class SerialCmd;

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

    Stage mStage = Stage::None;
    String mAnimalName;


#ifdef ONPC
    friend class DialogCollarSim;
    Animal* mAnimal;
#else

#endif


    Screen* mScreen = nullptr;
    GPS* mGPS = nullptr;
    Button* mBtnMain = nullptr;
    Led* mLed = nullptr;
    SerialCmd* mSerialCmd = nullptr;
    uint16_t mSequence = 0;
    uint32_t mAwakeningMillisScreen = 0;

    bool mIsSignal = false;
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
public:


#ifdef ONPC
    Collar(Animal* animal,
            const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray() );
    Protocol::Collar getPackageOut();
    QList<Protocol::Collar> getBoluses();
    QLine fenceClosestBorder();
    QPointF fenceClosestPoint();
    const Animal* animal() const;
    void sendToSerial(const char* str);
    QByteArray readFromSerial();
#else
    Collar();
#endif

    ~Collar();

    String& animalName();
    void onSetup();
    void onUpdate();
    void onSend();
    void onReceive(uint8_t* data, uint32_t size);
    inline Button* btnMain(){ return mBtnMain; }

    bool isFence(){ return mFencePointsCount > 0; }
    bool isInsideFence(){ return mIsInsideFence; }
    bool isGoingAwayFromFence(){ return mFenceIsGoingAway; }
    double fanceDistance(){ return mFenceDistance; }
    bool hasClosestFenceBorder(){ return nullptr != mFenceClosestBorder ; }
    Screen *screen() { return mScreen; }
    Led* led() { return mLed; }

    void restart();
    void sleep();

    int batteryLevel();

    int rssi(); // RSSI (Received Signal Strength Indicator) in dBm

    // SNR (Signal-to-Noise Ratio)
    int snr();

    GeoPoint gps();
    int satellites();

    int signalStrength(); // RSSI
};

#ifndef ONPC
    extern Collar* gCollar;
#endif

#endif // COLLAR_H
