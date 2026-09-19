#include "geometry.h"

bool Vector::isTowards(const Border &border) const {

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

Point Border::proj(const Point &p) {
    const Point pa = p - mBegin;
    double t = pa.dot(mD) / mLenSq; // find scale factor
    return Point(mBegin.mX + t * mD.mX, mBegin.mY + t * mD.mY);
}

bool Border::isInside(const Point &p){
    Point pa = mBegin - p;
    Point pb = mEnd - p;
    return (mLenSq > pa.dot(pa)) && (mLenSq > pb.dot(pb));
}

bool Border::isBetterVisible(Border &other, Point &fromPoint) {
    Point vb =  (fromPoint - mBegin).norm();
    Point ve =  (fromPoint - mEnd).norm();
    Point ovb = (fromPoint - other.mBegin).norm();
    Point ove = (fromPoint - other.mEnd).norm();
    double currentCross = vb.cross(ve);
    double otherCross =   ovb.cross(ove);
    return std::fabs(currentCross) > std::fabs(otherCross);
}

Point Point::norm() {
    double lenSqInv = 1.0 / std::sqrt(mX*mX + mY*mY);
    return Point( mX * lenSqInv, mY * lenSqInv);
}

Point Point::fromGeoPoint(const GeoPoint &center, const GeoPoint &pt) {
    // Approximate meters per degree
    const double metersPerDegLat = 111320.0;
    const double metersPerDegLon = 111320.0 * cos(deg2rad(center.mLat));

    double dLat = pt.mLat - center.mLat;
    double dLon = pt.mLon - center.mLon;

    return Point(dLon * metersPerDegLon, dLat * metersPerDegLat);
}

GeoPoint Point::toGeoPoint(const GeoPoint &center) {
    const double metersPerDegLat = 111320.0;

    double lat = center.mLat;
    double lon = center.mLon;

    double dLat = mY / metersPerDegLat;

    double metersPerDegLon = metersPerDegLat * cos(deg2rad(lat));
    double dLon = mX / metersPerDegLon;

    return GeoPoint(lat + dLat, lon + dLon);
}
