#ifndef GPS_H
#define GPS_H

class GPS
{
    bool mIsReady = false;
    float mLat = 0.0f;
    float mLon = 0.0f;

public:
    GPS();
    void setup();
    void onUpdate();

    inline bool isReady(){ return mIsReady; }

};

#endif // GPS_H
