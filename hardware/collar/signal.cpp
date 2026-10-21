#include "signal.h"

#include "../arduino.h"

Signal::Signal()
{

}

void Signal::execute()
{
    Serial.println(String(name()) + " not implemented.");
}
