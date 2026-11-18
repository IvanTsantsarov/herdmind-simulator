#ifndef LIGHTS_H
#define LIGHTS_H

#include "signaling.h"

class Lights : public Signaling
{
public:
    Lights(Collar* c);
    virtual ~Lights(){}
    const char* name() { return "Lights"; }
    void execute(uint8_t mode = 0);

};

#endif // LIGHTS_H
