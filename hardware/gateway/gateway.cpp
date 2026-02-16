#include "gateway.h"

#ifdef SIMULATION

Gateway::Gateway() : LoraDev( "Gateway", LoraDev::Profile::Gateway, GATEWAY_UPDATE_INTERVAL, GATEWAY_SEND_INTERVAL )
{

}

void Gateway::onUpdate()
{
    process();
}

void Gateway::onSend()
{
    sendPackage(nullptr, 0);
}

#else


#endif


void Gateway::process()
{

}
