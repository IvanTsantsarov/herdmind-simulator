#ifndef SIGNALING_H
#define SIGNALING_H

#include "../arduino.h"
#include "serialcmd.h"



class Collar;


// ********************************************
// ***** DO NOT USE THE NAME "Signal" !!! *****
// ********************************************

class Signaling
{
    Collar* mCollar = nullptr;
public:
    Signaling(Collar* c);
    virtual void execute() = 0;
    virtual const char* name() = 0;
    void respond(const String& msg);
    void respondCmd(SerialCmd::Cmd::Type t);
};

#endif // SIGNALING_H
