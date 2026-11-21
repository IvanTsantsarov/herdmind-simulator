#include "collar.h"
#include "gps.h"

#ifdef ONPC
#include "../../animal.h"
#include "../tools.h"
#endif

GeoPoint Collar::readGPS()
{
#ifdef ONPC
    if( mAnimal ) {
        QGeoCoordinate geoCoor = mAnimal->geoPos();
        return GeoPoint( geoCoor.latitude(), geoCoor.longitude());
    }
    return mGPS->pos();
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

void Collar::onSetupFence(uint8_t count, const GeoPoint& center, const uint8_t *offsetsPtr)
{
    if( mGPS->setupFence(count, center, offsetsPtr) ) {
        sendEvent(Protocol::Collar::Event::FenceOn, count);
    }else {
        sendEvent(Protocol::Collar::Event::FenceOff, 0);
    }
}


