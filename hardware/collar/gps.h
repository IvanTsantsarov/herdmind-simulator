#ifndef GPS_H
#define GPS_H

#include "geometry.h"

class GPS
{
    bool mIsReady = false;
    GeoPoint mPos;
    int mSatelites = 0;
    bool mIsConnection = false;
    bool mIsPowered = true;
    bool mIsHardware = false;

    void powerOff();
    void powerOn();

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
};

#endif // GPS_H
