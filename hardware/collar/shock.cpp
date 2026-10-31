#include "shock.h"

Shock::Shock(Collar *c) : Signaling(c) {}

void Shock::execute()
{
    respondCmd(SerialCmd::Cmd::Type::SIGE);
}
