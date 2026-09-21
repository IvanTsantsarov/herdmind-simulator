#include "button.h"

#ifdef SIMULATION
    #include "../tools.h"
#else
    #include <Arduino.h>
#endif


Button::Button(int pinNumber)
    : mPin(pinNumber)
{

}



void Button::setup()
{
    pinMode(mPin, INPUT_PULLUP);
}

void Button::update()
{
    mIsPressed = digitalRead(mPin) == LOW;
}
