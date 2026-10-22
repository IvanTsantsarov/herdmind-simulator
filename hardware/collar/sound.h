#ifndef SOUND_H
#define SOUND_H

#include "signaling.h"

class Sound : public Signaling
{
public:
    Sound(Collar* c);
    virtual ~Sound() { }
    const char* name() { return "Sound"; }
};

#endif // SOUND_H
