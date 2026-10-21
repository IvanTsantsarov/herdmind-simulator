#ifndef LIGHTS_H
#define LIGHTS_H

#include "signal.h"

class Lights : public Signal
{
public:
    Lights();
    const char* name() { return "Lights"; }

};

#endif // LIGHTS_H
