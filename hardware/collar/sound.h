#ifndef SOUND_H
#define SOUND_H

#include "signal.h"

class Sound : public Signal
{
public:
    Sound();
    const char* name() { return "Sound"; }
};

#endif // SOUND_H
