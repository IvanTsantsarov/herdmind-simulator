#ifndef SHOCK_H
#define SHOCK_H

#include "signal.h"

class Shock: public Signal
{
public:
    Shock();
    const char* name() { return "Shock"; }
};

#endif // SHOCK_H
