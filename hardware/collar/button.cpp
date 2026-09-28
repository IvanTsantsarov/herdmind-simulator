#include "button.h"

#include "../arduino.h"


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
