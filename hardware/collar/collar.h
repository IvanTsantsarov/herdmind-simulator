#ifndef COLLAR_H
#define COLLAR_H

#include <cmath>
#include <cstdint>

#include "screen.h"
#include "geometry.h"
#include "gps.h"
#include "../defines.h"
#include "../protocol.h"

// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 100

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 20000

#define COLLAR_MAX_GPS_POINTS 500


#ifdef SIMULATION
#include <QPointF>
#include <QLine>
#include "../loradev.h"

class Collar : public LoraDev
#else
class Collar
#endif
{
    enum struct Stage {
        None = 0,
        Setup = 1,
        Init = 2,
        Operate = 3
    };

    Stage mStage = Stage::None;
    String mAnimalName;


#ifdef SIMULATION
    const Animal* mAnimal;
#else

#endif

    Screen* mScreen = nullptr;
    GPS* mGPS = nullptr;
    uint16_t mSequence = 0;


    GeoPoint readGPS();
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

    bool mIsLoadingCleared = false;
public:

    String& animalName();
    void onSetup();
    void onUpdate();
    void onSend();
    void onReceive(uint8_t* data, uint32_t size);


#ifdef SIMULATION
    Collar(Animal* animal,
            const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray() );

    Protocol::Collar getPackageOut();;
    QList<Protocol::Collar> getBoluses();
    QLine fenceClosestBorder();

    QPointF fenceClosestPoint();


    const Animal* animal() const;

#else
    Collar();
#endif

    ~Collar();

    bool isFence();
    bool isInsideFence();
    bool isGoingAwayFromFence();
    double fanceDistance();
    bool hasClosestFenceBorder();
    Screen *screen();

};

#endif // COLLAR_H
