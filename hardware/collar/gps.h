#ifndef GPS_H
#define GPS_H

#include "geometry.h"

#include "../arduino.h"

class GPS
{
    GeoPoint mPos;
    Point mLastPoint;
    bool mIsReady = false;

    static RTC_DATA_ATTR float mFenceMaxDistance;
    static RTC_DATA_ATTR int mTrajectoryPointsCount;
    static RTC_DATA_ATTR GeoPoint mTrajectoryPoints[TRAJECTORY_MAX_POINTS];
    static RTC_DATA_ATTR GeoPoint mLastGeoPos;

    static void updateTrajectory(const GeoPoint& geoPt);

    static RTC_DATA_ATTR GeoPoint mGeoCenter;
    static RTC_DATA_ATTR int mFencePointsCount;
    static RTC_DATA_ATTR GeoPoint mFenceGeoPoints[VIRTUAL_FENCE_MAX_POINTS];
    Point mFencePoints[VIRTUAL_FENCE_MAX_POINTS];
    Border mFenceBorders[VIRTUAL_FENCE_MAX_POINTS];

    Border* mFenceClosestBorder = nullptr;
    Point mFenceClosestPoint;
    double mFenceClosestDistSq;
    double mFenceDistance;
    bool mFenceIsGoingAway = false;

    int mSatelites = 0;
    bool mIsConnection = false;
    bool mIsPowered = true;
    bool mIsHardware = false;

    void powerOff();



public:
    GPS();
    void setup(bool isWakeup);
    void onUpdate();

    inline bool isReady(){ return mIsReady; }
    inline const GeoPoint& pos(){ return mPos; }
    inline bool isConnection() { return mIsConnection; };
    inline int satelites() { return mSatelites; }

    void sleep();

    #ifdef ONPC
        void inject(GeoPoint gp, int sat);

    #endif

    bool isHardware(){ return mIsHardware; }

    bool isEmulated();
    inline static int trajectoryLen(){ return mTrajectoryPointsCount; }
    static GeoPoint tajectoryPoint(int index);


    ////// Fence support
    bool setupFence(uint8_t count,
                      const GeoPoint &center,
                      const uint8_t *offsetsPtr);


    inline bool hasFence(){ return mFencePointsCount > 0; }
    bool isInsideFence();

    inline bool isGoingAwayFromFence(){ return mFenceIsGoingAway; }
    inline double fenceDistance(){ return mFenceDistance; }
    inline bool hasClosestFenceBorder(){ return nullptr != mFenceClosestBorder ; }
    inline Border* fenceClosestBorder(){ return mFenceClosestBorder; }
    inline Point fenceClosestPoint(){ return mFenceClosestPoint; }
    inline bool isOutOfMaxDistance(){ return mFenceDistance > mFenceMaxDistance; }
};

#endif // GPS_H
