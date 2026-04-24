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
    Protocol::Collar package;
    QGeoCoordinate coord = mAnimal->geoLocation();

    if( !coord.isValid() ) {
        return;
    }

    package.encodeLat( coord.latitude() );
    package.encodeLon( coord.longitude() );
    package.mBattery = 100;
    // package.mEvent = ;

    sendPackage(package.toByteArray(), sizeof(Protocol::CollarByteArray));
}

void Collar::onReceive(uint8_t *data)
{
    const Protocol::Collar::Event event = static_cast<Protocol::Collar::Event>(data[0]);

    switch(event) {
    case Protocol::Collar::Event::SetupFence:
        break;
    }
}


#else
//////////////////////////////////////////////////////////////
/// Real conditions
//////////////////////////////////////////////////////////////


#endif // SIMULATION
