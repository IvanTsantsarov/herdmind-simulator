#include "lights.h"
#include "../arduino.h"

Lights::Lights(Collar *c) : Signaling(c)
{

}

void Lights::execute()
{
    respondCmd(SerialCmd::Cmd::Type::SIGL);
}
