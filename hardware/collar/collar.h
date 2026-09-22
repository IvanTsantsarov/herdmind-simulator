#ifndef COLLAR_H
#define COLLAR_H

#include <cmath>
#include <cstdint>

#include "geometry.h"

#include "../defines.h"
#include "../protocol.h"

#ifdef SIMULATION
    #include "../tools.h"
#else
    #include <Arduino.h>
#endif

// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 100

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 20000

#define COLLAR_MAX_GPS_POINTS 500

class GPS;
class Screen;
class Button;
class Led;

#ifdef SIMULATION
#include <QPointF>
#include <QLine>
#include "../loradev.h"

class Collar : public LoraDev
#else
class Collar
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


#ifdef SIMULATION
    friend class DialogCollarSim;
    Animal* mAnimal;
#else

#endif

    Screen* mScreen = nullptr;
    GPS* mGPS = nullptr;
    Button* mBtnMain = nullptr;
    Led* mLed = nullptr;
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
    void createObjects();
    void updateScreenLoading(bool isFlush = true);
    void updateScreenNormal(bool isFlush = true);
public:


#ifdef SIMULATION
    Collar(Animal* animal,
            const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray() );
    Protocol::Collar getPackageOut();
    QList<Protocol::Collar> getBoluses();
    QLine fenceClosestBorder();
    QPointF fenceClosestPoint();
    const Animal* animal() const;
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

    void sleep();
};

#ifndef SIMULATION
    extern Collar* gCollar;
#endif

#endif // COLLAR_H
