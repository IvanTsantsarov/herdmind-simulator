#include "collar.h"
#include "../tools.h"

#ifdef SIMULATION
#include "../../animal.h"
#endif

#ifdef SIMULATION
//////////////////////////////////////////////////////////////
/// Simulation
//////////////////////////////////////////////////////////////

Collar::GeoPoint Collar::readGPS()
{
    QGeoCoordinate geoCoor = mAnimal->geoPos();
    return GeoPoint( geoCoor.latitude(), geoCoor.longitude());
}


Collar::Collar( Animal* animal,
                const QByteArray &devEUI,
                const QByteArray& appKey)
    : LoraDev(QString("%1 Collar").arg(animal->name()), LoraDev::Profile::Collar,
            COLLAR_UPDATE_INTERVAL, COLLAR_SEND_INTERVAL,
              devEUI, appKey), mAnimal(animal)
{
}


void Collar::onUpdate()
{
    GeoPoint geoPt = readGPS();

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

void Collar::onSend()
{
    Protocol::Collar package;

    package.mEvent = Protocol::Collar::Event::Package;
    GeoPoint coord = readGPS();
    package.encodeLat( coord.mLat );
    package.encodeLon( coord.mLon );
    package.mBattery = 100;

    sendPackage(package.toByteArray(), sizeof(Protocol::CollarByteArray));
}

void Collar::onReceive(uint8_t *data, uint32_t size)
{
    int offset = 0;
    const Protocol::Collar::Event event = static_cast<Protocol::Collar::Event>(data[offset++]);

    switch(event) {
    case Protocol::Collar::Event::SetupFence:
    {
        uint32_t count = data[offset]; offset += 1;
        double lat = Protocol::decodeLat( Protocol::readUint32(data, offset )); offset += sizeof(uint32_t);
        double lon = Protocol::decodeLon( Protocol::readUint32(data, offset)); offset += sizeof(uint32_t);
        onSetupFence( count, GeoPoint(lat, lon), data + offset );
    }
        break;
    case Protocol::Collar::Event::Light: break;
    case Protocol::Collar::Event::Sound: break;
    case Protocol::Collar::Event::Shock: break;
    // default: assert(0);
    case Protocol::Collar::Event::None:
    case Protocol::Collar::Event::Package:
    case Protocol::Collar::Event::FenceOn:
    case Protocol::Collar::Event::FenceOff:
        break;
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

        qInfo() << QString::number(geoPt.mLat, 'f', 6) << QString::number(geoPt.mLon, 'f', 6); // trash
        mFenceGeoPoints[ptIndex] = geoPt;
        mFencePoints[ptIndex] = Point::fromGeoPoint(center, geoPt);
    }

    mFenceBordersCount = mFencePointsCount/2;
    for( auto bIndex = 0; bIndex < mFenceBordersCount; bIndex ++ ) {
        mFenceBorders[bIndex] = Border(mFencePoints[bIndex*2], mFencePoints[bIndex*2+1]);
    }

    sendEvent(Protocol::Collar::Event::FenceOn, mFencePointsCount);
}

void Collar::sendEvent(Protocol::Collar::Event event, uint32_t value)
{
    uint8_t buffer[1 + sizeof(uint32_t)];
    buffer[0] = static_cast<uint8_t>(event);
    Protocol::writeUint32(value, buffer, 1);
    sendPackage(buffer, sizeof(buffer));
}


#else
//////////////////////////////////////////////////////////////
/// Real conditions
//////////////////////////////////////////////////////////////


#endif // SIMULATION


void Collar::testFence()
{
    mIsInsideFence = false;

    Point p = mLastPoint;

    for( auto i = 0; i < mFenceBordersCount; i ++) {
        Border& border = mFenceBorders[i];

        if( border.isOn(mLastPoint)) {
            mIsInsideFence = true;
            break;
        }

        const Point& a = border.begin();
        const Point& b = border.end();

        bool intersects =
            ((a.mY > mLastPoint.mY) != (b.mY > p.mY)) &&
            (p.mX < (b.mX - a.mX) * (p.mY - a.mY) /
                            (b.mY - a.mY) + a.mX);

        if (intersects)
            mIsInsideFence = !mIsInsideFence;
    }
}
