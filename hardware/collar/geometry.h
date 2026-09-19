#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdlib.h>
#include <cmath>

#define M_PI 3.14159265358979323846
#define deg2rad(__deg__) (__deg__ * M_PI / 180.0)
#define rad2deg(__rad__) (__rad__ * 180.0 / M_PI)
#define MIN_DOUBLE_VALUE 1e-9

struct GeoPoint {
    float mLat, mLon, mAlt;
    inline GeoPoint(): mLat{0.0}, mLon{0.0}{};
    inline GeoPoint(float lat, float lon): mLat{lat}, mLon{lon}{};
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

    Point norm();

    static Point fromGeoPoint(const GeoPoint& center, const GeoPoint& pt);

    GeoPoint toGeoPoint(const GeoPoint& center);
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
    Point proj(const Point& p);

    bool isInside(const Point& p);

    // Border visibility reliase on angle between
    // point of view and both start and end point of the fence border
    bool isBetterVisible(Border& other, Point& fromPoint);
};

struct Vector {
    Point mBegin, mEnd;
    Vector( const Point& begin, const Point& end): mBegin{begin}, mEnd{end}{}

    // First, calculate the common denominator.
    bool isTowards(const Border& border) const;
};



#endif // GEOMETRY_H
