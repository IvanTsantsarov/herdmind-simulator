#ifndef SHOCK_H
#define SHOCK_H

#include "signaling.h"

class Shock: public Signaling
{
public:
    Shock(Collar* c);
    virtual ~Shock(){ }
    const char* name() { return "Shock"; }
    void execute();
};

#endif // SHOCK_H
