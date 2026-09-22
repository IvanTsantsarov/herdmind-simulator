#include "collar.h"
#include "gps.h"

#ifdef SIMULATION
#include "../../animal.h"
#include "../tools.h"
#endif

GeoPoint Collar::readGPS()
{
#ifdef SIMULATION
    QGeoCoordinate geoCoor = mAnimal->geoPos();
    return GeoPoint( geoCoor.latitude(), geoCoor.longitude());
#else
    return mGPS->pos();
#endif
}

void Collar::printGPS()
{
    GeoPoint pos = mGPS->pos();
    Serial.print("Pos: ");
    Serial.print(pos.mLat, 6);
    Serial.print(", ");
    Serial.print(pos.mLon, 6);
    Serial.print(", ");
    Serial.println(pos.mAlt, 3);
    Serial.print("SAT: ");
    Serial.println(mGPS->satelites());
}


void Collar::updateTrajectory(const GeoPoint& geoPt)
{
    // increment trajectory buffer counter
    mTrajectoryPointsCount ++;

    // if trajectory point counter exceeds maximum count
    // shift left the whole array with one element
    if( mTrajectoryPointsCount >= COLLAR_MAX_GPS_POINTS) {
        for( auto i = 1; i < COLLAR_MAX_GPS_POINTS; i++) {
            mTrajectoryPoints[i-1] = mTrajectoryPoints[i];
        }
        mTrajectoryPointsCount = COLLAR_MAX_GPS_POINTS;
    }

    mLastGeoPos = geoPt;
    mLastPoint = Point::fromGeoPoint(mGeoCenter, mLastGeoPos);
    if( isFence()) {
        testFence();
    }

    // add current geo location to the end of the tragectory points buffer
    mTrajectoryPoints[mTrajectoryPointsCount-1] = geoPt;
    if( mTrajectoryPointsCount < 2 ) {
        return;
    }

}

void Collar::onSetupFence(uint8_t count, const GeoPoint& center, const uint8_t *offsetsPtr)
{
    mFencePointsCount = count;
    if( !mFencePointsCount) {
        sendEvent(Protocol::Collar::Event::FenceOff, 0);
        return;
    }

    mGeoCenter = center;
    int coordsIndex = 0;
    for( uint8_t ptIndex = 0; ptIndex < count; ptIndex ++, coordsIndex += 4 ) {
        int16_t offsetLat = Protocol::readInt16(offsetsPtr, coordsIndex);
        int16_t offsetLon = Protocol::readInt16(offsetsPtr, coordsIndex + 2);
        GeoPoint geoPt(
            Protocol::decodeCoordOffset(offsetLat, center.mLat ),
            Protocol::decodeCoordOffset(offsetLon, center.mLon ) );

        // qInfo() << QString::number(geoPt.mLat, 'f', 6) << QString::number(geoPt.mLon, 'f', 6); // trash
        mFenceGeoPoints[ptIndex] = geoPt;
        mFencePoints[ptIndex] = Point::fromGeoPoint(center, geoPt);
    }

    for( auto bIndex = 0; bIndex < mFencePointsCount; bIndex ++ ) {
        int nextIndex = bIndex + 1;
        if( nextIndex >= mFencePointsCount) {
            nextIndex = 0;
        }
        mFenceBorders[bIndex] = Border(mFencePoints[bIndex], mFencePoints[nextIndex]);
    }

    sendEvent(Protocol::Collar::Event::FenceOn, mFencePointsCount);
}


void Collar::testFence()
{
    mIsInsideFence = false;

    Point p = mLastPoint;

    // Find closest border and poind
    double closestDistSq = MAXFLOAT;
    int closestBorderIndex = -1;
    Point closestProj;

    // Test for inside/outside of the fence
    for( auto i = 0; i < mFencePointsCount; i ++) {
        Border& border = mFenceBorders[i];

        if( border.isOn(mLastPoint)) {
            closestDistSq = 0.0;
            mIsInsideFence = true;
            closestBorderIndex = i;
            closestProj = mLastPoint;
            break;
        }

        const Point& a = border.begin();
        const Point& b = border.end();

        bool intersects =
            ((a.mY > p.mY) != (b.mY > p.mY)) &&
            (p.mX < (b.mX - a.mX) * (p.mY - a.mY) /
                            (b.mY - a.mY) + a.mX);

        if (intersects) {
            mIsInsideFence = !mIsInsideFence;
        }

        // find closest side by the projection point
        Point proj = border.proj(p);
        double distSq = p.distSq(proj);
        if( distSq < closestDistSq && border.isInside(proj)) {
            closestProj = proj;
            closestDistSq = distSq;
            closestBorderIndex = i;
        }
    }

    // if no closest border found
    // reset search for closest end point
    // else try to find closest end point
    if( closestBorderIndex < 0 ) {
        closestDistSq = MAXFLOAT;
    }

    // Fist closest border end point then projection point
    int closestPointIndex = -1;
    for( auto i = 0; i < mFencePointsCount; i ++) {
        Point& bp = mFencePoints[i];
        double distSq = bp.distSq(p);
        if( distSq < closestDistSq ) {
            closestDistSq = distSq;
            closestPointIndex = i;
        }
    }

    // If closer end point of the border found,
    // then find which border is better visible
    // from the current animal point
    if( closestPointIndex >= 0 ) {
        int borderRightIndex = closestPointIndex;
        int borderLeftIndex = closestPointIndex - 1;
        if( borderLeftIndex < 0 ) {
            borderLeftIndex = mFencePointsCount - 1;
        }

        Border& borderRight = mFenceBorders[borderRightIndex];
        Border& borderLeft = mFenceBorders[borderLeftIndex];

        closestBorderIndex = borderLeft.isBetterVisible(borderRight, p) ?
                                 borderLeftIndex : borderRightIndex;
    }

    if(closestBorderIndex >= 0) {
        mFenceIsGoingAway = closestDistSq > mFenceClosestDistSq;
        mFenceClosestDistSq = closestDistSq;
        mFenceClosestBorder = &mFenceBorders[closestBorderIndex];
        mFenceClosestPoint = closestProj;
        mFenceDistance = std::sqrt(mFenceClosestDistSq);
    }else {
        assert(0);
    }
}
