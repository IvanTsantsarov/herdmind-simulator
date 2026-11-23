#include "lights.h"
#include "../arduino.h"

Lights::Lights(Collar *c) : Signaling(c)
{

}

void Lights::execute(uint8_t mode)
{
    respondCmd(CLI::Cmd::Type::SIGL);
    // delay(3000);
}
