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

    // add current geo location to the end of the tragectory points buffer
    mTrajectoryPoints[mTrajectoryPointsCount-1] = geoPt;
    if( mTrajectoryPointsCount < 2 ) {
        return;
    }


    if( isFence()) {
        Point pt = Point::fromGeoPoint(mGeoCenter, geoPt);

        for( auto bIndex = 0; bIndex < mFenceBordersCount; bIndex ++) {
            // TODO: implement fence signaling

        }
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
        uint32_t count = Protocol::readUint32(data, offset); offset += sizeof(uint32_t);
        double lat = Protocol::decodeLat( Protocol::readUint32(data, offset )); offset += sizeof(uint32_t);
        double lon = Protocol::decodeLon( Protocol::readUint32(data, offset)); offset += sizeof(uint32_t);
        uint32_t* pointsData = reinterpret_cast<uint32_t*>(data + offset);
        onSetupFence( count, GeoPoint(lat, lon), pointsData );
    }
        break;
    case Protocol::Collar::Event::Light: break;
    case Protocol::Collar::Event::Sound: break;
    case Protocol::Collar::Event::Shock: break;
    // default: assert(0);
    }
}

void Collar::onSetupFence(uint32_t count, const GeoPoint& center, uint32_t *coordsPtr)
{
    mFencePointsCount = count;
    if( !mFencePointsCount) {
        sendEvent(Protocol::Collar::Event::FenceOff, 0);
        return;
    }

    mGeoCenter = center;
    for( uint32_t ptIndex = 0; ptIndex < count; ptIndex ++ ) {
        int coordsIndex = ptIndex*2;
        GeoPoint geoPt(
            Protocol::decodeLat(coordsPtr[coordsIndex]),
            Protocol::decodeLon(coordsPtr[coordsIndex + 1]) );

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
