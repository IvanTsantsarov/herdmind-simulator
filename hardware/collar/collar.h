#ifndef COLLAR_H
#define COLLAR_H

#include <cmath>
#include <cstdint>

#include "../defines.h"

// interval for reading the sensors
#define COLLAR_UPDATE_INTERVAL 100

// interval for sending data to collars/gateways
#define COLLAR_SEND_INTERVAL 20000

#define COLLAR_MAX_GPS_POINTS 500

#define MIN_DOUBLE_VALUE 1e-9

struct CollarData;

#ifdef SIMULATION
#include <QPointF>
#include <QLine>
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
        inline Point(double x, double y): mX{x}, mY{y}{};

        inline Point operator +(const Point& pt) const{
            return Point(mX + pt.mX, mY + pt.mY);
        }

        inline Point operator - (const Point& pt) const {
            return Point(mX - pt.mX, mY - pt.mY);
        }

        inline double dot(const Point& pt) const {
            return mX*pt.mX + mY*pt.mY;
        }

        inline double distSq(const Point& pt) const {
            Point d = *this - pt;
            return d.dot(d);
        }


        inline double cross(const Point& pt) const {
            return mX*pt.mY - mY*pt.mX;
        }

        Point norm() {
            double lenSqInv = 1.0 / std::sqrt(mX*mX + mY*mY);
            return Point( mX * lenSqInv, mY * lenSqInv);
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

    class Border {
        Point mBegin, mEnd;

        Point mD;
        double mLenSq = 0.0;
        bool mIsFlat = false;
    public:
        inline Border() { }
        inline Border(const Point& b, const Point& e): mBegin{b}, mEnd{e}
        {
            mD = mEnd - mBegin;
            mIsFlat = (mD.mY < 0.0f ? -mD.mY : mD.mY) < MIN_DOUBLE_VALUE;
            mLenSq = mD.dot(mD);
        };

        inline Point begin() const { return mBegin; }
        inline Point end() const { return mEnd; }
        inline Point d() const { return mD; }
        inline double lenSq() const { return mLenSq; }

        inline bool isOn(const Point& pt) const {
            Point d = pt - mBegin;

            if( fabs(mD.cross(d)) > MIN_DOUBLE_VALUE ) {
                return false;
            }

            double dot = d.dot( mD );
            if( dot < 0 ) {
                return false;
            }

            return dot <= mLenSq;
        }

        // get an ortogonal projection of a given
        // point "p" on the border line
        Point proj(const Point& p) {
            const Point pa = p - mBegin;
            double t = pa.dot(mD) / mLenSq; // find scale factor
            return Point(mBegin.mX + t * mD.mX, mBegin.mY + t * mD.mY);
        }

        bool isInside(const Point& p){
            Point pa = mBegin - p;
            Point pb = mEnd - p;
            return (mLenSq > pa.dot(pa)) && (mLenSq > pb.dot(pb));
        }

        // Border visibility reliase on angle between
        // point of view and both start and end point of the fence border
        bool isBetterVisible(Border& other, Point& fromPoint) {
            Point vb =  (fromPoint - mBegin).norm();
            Point ve =  (fromPoint - mEnd).norm();
            Point ovb = (fromPoint - other.mBegin).norm();
            Point ove = (fromPoint - other.mEnd).norm();
            double currentCross = vb.cross(ve);
            double otherCross =   ovb.cross(ove);
            return std::fabs(currentCross) > std::fabs(otherCross);
        }

    };

    struct Vector {
        Point mBegin, mEnd;
        Vector( const Point& begin, const Point& end): mBegin{begin}, mEnd{end}{}

        // First, calculate the common denominator.
        bool isTowards(const Border& border) const {

            Point d1 = mBegin - mEnd;
            Point d2 = border.d();

            double d = d1.mX * d2.mY - d1.mY * d2.mX;

            if( std::fabs(d) < MIN_DOUBLE_VALUE ) {
                return false;
            }

            double k = 1.0 / d;

            double cross1 = mBegin.cross(mEnd);
            double cross2 = border.begin().cross(border.end());

            double px = ( cross1*d2.mX - d1.mX*cross2 ) * k;
            double py = ( cross1*d2.mY - d1.mY*cross2 ) * k;

            Point common(px, py);

            return mEnd.distSq(common) < mBegin.distSq(common);
        }
    };


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

public:


private:
    void testFence();
#ifdef SIMULATION
    Animal* mAnimal;
#else
    Collar();
#endif
    Protocol::Collar mPackage;

    void onUpdate();
    void onSend();
    void onReceive(uint8_t* data, uint32_t size);

    void onSetupFence(uint8_t count,
                      const GeoPoint &center,
                      const uint8_t *offsetsPtr);

    void sendEvent(Protocol::Collar::Event event, uint32_t value);
public:

#ifdef SIMULATION
    Collar(Animal* animal,
            const QByteArray &devEUI = QByteArray(),
           const QByteArray& appKey = QByteArray() );

    Protocol::Collar getPackageOut(){ return mPackage; };
    QList<Protocol::Collar> getBoluses();
    QLine fenceClosestBorder() {
        if( !mFenceClosestBorder ) {
            return QLine();
        }

        return QLine(mFenceClosestBorder->begin().mX,
                     mFenceClosestBorder->begin().mY,
                     mFenceClosestBorder->end().mX,
                     mFenceClosestBorder->end().mY);
    }

    QPointF fenceClosestPoint() {
        if( !mFenceClosestBorder ) {
            return QPointF();
        }

        return QPointF(mFenceClosestPoint.mX, mFenceClosestPoint.mY);
    }

#endif

    inline bool isFence(){ return mFencePointsCount > 0; }
    inline bool isInsideFence(){ return mIsInsideFence; }
    inline bool isGoingAwayFromFence(){ return mFenceIsGoingAway; }
    inline double fanceDistance(){ return mFenceDistance; }
    inline bool hasClosestFenceBorder(){ return nullptr != mFenceClosestBorder ; }

};

struct CollarData {
    Protocol::Collar mCollar;
    QList<Protocol::Bolus> mBoluses;
};


#endif // COLLAR_H
