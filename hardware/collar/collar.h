#ifndef COLLAR_H
#define COLLAR_H

#include <cstdint>

// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 5000

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 20000

#define COLLAR_MAX_FENCE_POINTS 10
#define COLLAR_MAX_FENCE_BORDERS (COLLAR_MAX_FENCE_POINTS/2)

#define COLLAR_MAX_GPS_POINTS 500

#define MIN_DOUBLE_VALUE 0.00001

struct CollarData;

#ifdef SIMULATION
#include <QPointF>
#include "../loradev.h"
#include "../protocol.h"

#define M_PI 3.14159265358979323846
#define deg2rad(__deg__) (__deg__ * M_PI / 180.0)
#define rad2deg(__rad__) (__rad__ * 180.0 / M_PI)

class Collar : public LoraDev

#else
class Collar
#endif
{
    uint16_t mSequence = 0;

    struct GeoPoint {
        double mLat, mLon;
        inline GeoPoint(): mLat{0.0}, mLon{0.0}{};
        inline GeoPoint(double lat, double lon): mLat{lat}, mLon{lon}{};
    };

    struct Point {
        double mX, mY;
        inline Point(): mX{0.0}, mY{0.0}{};
        inline Point(float x, float y): mX{x}, mY{y}{};

        inline Point operator +(const Point& pt) const{
            return Point(mX + pt.mX, mY + pt.mY);
        }

        inline Point operator - (const Point& pt) const {
            return Point(mX - pt.mX, mY - pt.mY);
        }

        double dot(const Point& pt) const {
            return mX*pt.mX + mY*pt.mY;
        }

        double distSq(const Point& pt) const {
            Point d = *this - pt;
            return d.cross(d);
        }


        double cross(const Point& pt) const {
            return mX*pt.mY - mY*pt.mX;
        }

        static Point fromGeoPoint(const GeoPoint& center, const GeoPoint& pt) {
            // Approximate meters per degree
            const double metersPerDegLat = 111320.0;
            const double metersPerDegLon = 111320.0 * cos(deg2rad(center.mLat));

            double dLat = pt.mLat - center.mLat;
            double dLon = pt.mLon - center.mLon;

            return Point(dLon * metersPerDegLon, dLat * metersPerDegLat);
        }

        GeoPoint toGeoPoint(const GeoPoint& center) {
            const double metersPerDegLat = 111320.0;

            double lat = center.mLat;
            double lon = center.mLon;

            double dLat = mY / metersPerDegLat;

            double metersPerDegLon = metersPerDegLat * cos(deg2rad(lat));
            double dLon = mX / metersPerDegLon;

            return GeoPoint(lat + dLat, lon + dLon);
        }
    };

    struct Border {
        Point mBegin, mEnd;
        float mA, mB, mK;
        Border() :  mA{0.0}, mB{0.0}{}
        inline Border(const Point& b, const Point& e): mBegin{b}, mEnd{e}
        {
            Point dd = mBegin - mEnd;
            mA = dd.mY / dd.mX;
            mB = mBegin.mY - mA * mBegin.mX;
            mK = 1.0 / sqrt(mA*mA + 1);
        };

        inline bool isInside(const Point& pt) {
            return mA * pt.mX + pt.mY > 0.0f;
        }

        double distance(const Point& pt) {
            return (mA*pt.mX - pt.mY + mB) * mK;
        }
    };

    struct Vector {
        Point mBegin, mEnd;
        Vector( const Point& begin, const Point& end): mBegin{begin}, mEnd{end}{}

        // First, calculate the common denominator.
        bool isTowards(const Border& border) {

            Point d1 = mBegin - mEnd;
            Point d2 = border.mBegin - border.mEnd;

            double d = d1.mX * d2.mY - d1.mY * d2.mX;

            if( abs(d) < MIN_DOUBLE_VALUE ) {
                return false;
            }

            double k = 1.0 / d;

            double cross1 = mBegin.cross(mEnd);
            double cross2 = border.mBegin.cross(border.mEnd);

            double px = ( cross1*d2.mX - d1.mX*cross2 ) * k;
            double py = ( cross1*d2.mY - d1.mY*cross2 ) * k;

            Point common(px, py);

            return mEnd.distSq(common) < mBegin.distSq(common);
        }
    };


    GeoPoint readGPS();

    float mFenceDistanceSound1;
    float mFenceDistanceSound2;
    float mFenceDistanceSoundShock;

    GeoPoint mGeoCenter;
    int mFencePointsCount = 0;
    GeoPoint mFenceGeoPoints[COLLAR_MAX_FENCE_POINTS];
    Point mFencePoints[COLLAR_MAX_FENCE_POINTS];

    int mFenceBordersCount = 0;
    Border mFenceBorders[COLLAR_MAX_FENCE_BORDERS];

    int mTrajectoryPointsCount = 0;
    GeoPoint mTrajectoryPoints[COLLAR_MAX_GPS_POINTS];

public:
    inline bool isFence(){ return mFencePointsCount > 0; }

private:
#ifdef SIMULATION
    Animal* mAnimal;
#else
    Collar();
#endif
    Protocol::Collar mPackage;

    void onUpdate();
    void onSend();
    void onReceive(uint8_t* data, uint32_t size);

    void onSetupFence(uint32_t count,
                      const GeoPoint &center,
                      uint32_t *coordsPtr);

    void sendEvent(Protocol::Collar::Event event, uint32_t value);
public:

#ifdef SIMULATION

    Collar(Animal* animal,
            const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray() );

    Protocol::Collar getPackageOut(){ return mPackage; };
    QList<Protocol::Collar> getBoluses();
#endif


};

struct CollarData {
    Protocol::Collar mCollar;
    QList<Protocol::Bolus> mBoluses;
};


#endif // COLLAR_H
