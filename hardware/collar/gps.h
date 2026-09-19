#ifndef GPS_H
#define GPS_H

#include "geometry.h"

class GPS
{
    bool mIsReady = false;
    GeoPoint mPos;
    int mSatelites = 0;
    bool mIsConnection = false;

public:
    GPS();
    void setup();
    void onUpdate();

    inline bool isReady(){ return mIsReady; }
    inline const GeoPoint& pos(){ return mPos; }
    inline bool isConnection() { return mIsConnection; };
    inline int satelites() { return mSatelites; }


};

#endif // GPS_H
