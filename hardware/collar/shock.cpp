#include "shock.h"

Shock::Shock(Collar *c) : Signaling(c) {}

void Shock::execute(uint8_t mode)
{
    respondCmd(CLI::Cmd::Type::SIGE);
    // delay(3000);
}
