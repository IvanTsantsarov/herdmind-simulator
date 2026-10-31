#ifndef LIGHTS_H
#define LIGHTS_H

#include "signaling.h"

class Lights : public Signaling
{
public:
    Lights(Collar* c);
    virtual ~Lights(){}
    const char* name() { return "Lights"; }
    void execute();

};

#endif // LIGHTS_H
