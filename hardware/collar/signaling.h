#ifndef SIGNALING_H
#define SIGNALING_H

#include "../arduino.h"



class Collar;


// ********************************************
// ***** DO NOT USE THE NAME "Signal" !!! *****
// ********************************************

class Signaling
{
    Collar* mCollar = nullptr;
public:
    Signaling(Collar* c);
    virtual void execute();
    virtual const char* name() = 0;
    void respond(const String& msg);
};

#endif // SIGNALING_H
