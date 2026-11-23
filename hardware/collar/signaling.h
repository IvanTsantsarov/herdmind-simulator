#ifndef SIGNALING_H
#define SIGNALING_H

#include "../arduino.h"
#include "cli.h"



class Collar;


// ********************************************
// ***** DO NOT USE THE NAME "Signal" !!! *****
// ********************************************

class Signaling
{
    Collar* mCollar = nullptr;
public:
    Signaling(Collar* c);
    virtual void execute(uint8_t mode) = 0;
    virtual const char* name() = 0;
    void respond(const String& msg);
    void respondCmd(CLI::Cmd::Type t);
};

#endif // SIGNALING_H
