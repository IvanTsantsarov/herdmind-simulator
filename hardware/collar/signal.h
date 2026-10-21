#ifndef SIGNAL_H
#define SIGNAL_H

class Signal
{
public:
    Signal();
    virtual void execute();
    virtual const char* name() = 0;
};

#endif // SIGNAL_H
