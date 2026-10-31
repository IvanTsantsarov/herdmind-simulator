#include "signaling.h"
#include "collar.h"

#include "../arduino.h"

Signaling::Signaling(Collar* c) : mCollar(c)
{

}

void Signaling::respond(const String &msg)
{
    mCollar->serialRespond(msg);
}

void Signaling::respondCmd(SerialCmd::Cmd::Type t)
{
    mCollar->serialCmd(t);
}
