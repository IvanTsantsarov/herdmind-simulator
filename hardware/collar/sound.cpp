#include "sound.h"

Sound::Sound(Collar *c) : Signaling(c) {}

void Sound::execute()
{
    respondCmd(SerialCmd::Cmd::Type::SIGS);
}
