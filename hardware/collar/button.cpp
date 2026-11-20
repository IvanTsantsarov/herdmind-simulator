#include "button.h"

#include "../arduino.h"


Button::Button(int pinNumber)
    : mPin(pinNumber)
{

}

bool gButtonDown = false;
void gButtonDownInterrupt() {
    gButtonDown = true;
}


bool gButtonUp = false;
void gButtonUpInterrupt() {
    gButtonUp = true;
}


void Button::setup(onButtonClickCallback clickCallback)
{
    pinMode(mPin, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(mPin),
        gButtonDownInterrupt,
        FALLING
        );

    attachInterrupt(
        digitalPinToInterrupt(mPin),
        gButtonUpInterrupt,
        RISING
        );
}

void Button::update()
{
    mIsPressed = digitalRead(mPin) == LOW;

    if( gButtonDown ) {
        mMsecPressStart = millis();
        gButtonDown = false;
    }

    if( gButtonUp ) {
        mMsecPressed = millis() - mMsecPressStart;
        gButtonUp = false;
    }
}
