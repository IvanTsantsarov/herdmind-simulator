#include "collar.h"
#include "../tools.h"

#ifdef SIMULATION
#include "../../animal.h"
#endif




#ifdef SIMULATION
//////////////////////////////////////////////////////////////
/// Simulation
//////////////////////////////////////////////////////////////

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
}

void Collar::onSend()
{
    PackageOut p;
    QGeoCoordinate coord = mAnimal->geoLocation();

    if( !coord.isValid() ) {
        return;
    }

    p.latitude = Tools::encodeLat( coord.latitude() );
    p.longitude = Tools::encodeLon( coord.longitude() );
    p.battery = 100;
    p.event = 0;
    p.seq = mSequence++;
    sendPackage(&p, sizeof(p));
}


#else
//////////////////////////////////////////////////////////////
/// Real conditions
//////////////////////////////////////////////////////////////


#endif // SIMULATION
