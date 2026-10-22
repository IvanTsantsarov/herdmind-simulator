#include "signaling.h"
#include "collar.h"

#include "../arduino.h"

Signaling::Signaling(Collar* c) : mCollar(c)
{

}

void Signaling::execute()
{
    respond(String(name()) + " not implemented.");
}


void Signaling::respond(const String &msg)
{
    mCollar->serialRespond(msg);
}
