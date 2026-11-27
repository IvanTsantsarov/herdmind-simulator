#include "button.h"

#include "../arduino.h"


Button* Button::InterruptsTable[BUTTON_INTERRUPTS_COUNT] = {0};


template<> void Button::interruptDown<0>() {   Button::InterruptsTable[0]->onDown(); }
template<> void Button::interruptDown<1>() {   Button::InterruptsTable[1]->onDown(); }
template<> void Button::interruptDown<2>() {   Button::InterruptsTable[2]->onDown(); }
template<> void Button::interruptDown<3>() {   Button::InterruptsTable[3]->onDown(); }
template<> void Button::interruptDown<4>() {   Button::InterruptsTable[4]->onDown(); }
template<> void Button::interruptDown<5>() {   Button::InterruptsTable[5]->onDown(); }
template<> void Button::interruptDown<6>() {   Button::InterruptsTable[6]->onDown(); }
template<> void Button::interruptDown<7>() {   Button::InterruptsTable[7]->onDown(); }
template<> void Button::interruptDown<9>() {   Button::InterruptsTable[9]->onDown(); }

template<> void Button::interruptDown<10>() { Button::InterruptsTable[10]->onDown(); }
template<> void Button::interruptDown<11>() { Button::InterruptsTable[11]->onDown(); }
template<> void Button::interruptDown<12>() { Button::InterruptsTable[12]->onDown(); }
template<> void Button::interruptDown<13>() { Button::InterruptsTable[13]->onDown(); }
template<> void Button::interruptDown<14>() { Button::InterruptsTable[14]->onDown(); }
template<> void Button::interruptDown<15>() { Button::InterruptsTable[15]->onDown(); }
template<> void Button::interruptDown<16>() { Button::InterruptsTable[16]->onDown(); }
template<> void Button::interruptDown<17>() { Button::InterruptsTable[17]->onDown(); }
template<> void Button::interruptDown<19>() { Button::InterruptsTable[19]->onDown(); }

template<> void Button::interruptDown<20>() { Button::InterruptsTable[20]->onDown(); }
template<> void Button::interruptDown<21>() { Button::InterruptsTable[21]->onDown(); }
template<> void Button::interruptDown<22>() { Button::InterruptsTable[22]->onDown(); }
template<> void Button::interruptDown<23>() { Button::InterruptsTable[23]->onDown(); }
template<> void Button::interruptDown<24>() { Button::InterruptsTable[24]->onDown(); }
template<> void Button::interruptDown<25>() { Button::InterruptsTable[25]->onDown(); }
template<> void Button::interruptDown<26>() { Button::InterruptsTable[26]->onDown(); }
template<> void Button::interruptDown<27>() { Button::InterruptsTable[27]->onDown(); }
template<> void Button::interruptDown<29>() { Button::InterruptsTable[29]->onDown(); }

template<> void Button::interruptDown<30>() { Button::InterruptsTable[30]->onDown(); }
template<> void Button::interruptDown<31>() { Button::InterruptsTable[31]->onDown(); }



void Button::onUp(){
    mIsDown = false;
    mMsecEvent = millis();
}

void Button::onDown(){
    mIsDown = true;
    mIsEvent = true;
    mMsecEvent = millis();
}

Button::Button(int pinNumber)
    : mPin(pinNumber)
{
    mMsecEvent = millis();
}


void Button::setup(Button::EventCallback eventCallback, Collar* c)
{
    // HIGH when unpressed and drops to LOW when pressed.
    pinMode(mPin, INPUT_PULLUP);

    mEventCallback = eventCallback;
    mCollar = c;

    int intIdx = digitalPinToInterrupt(mPin);

    assert(intIdx >= 0 && intIdx < BUTTON_INTERRUPTS_COUNT);

    Button::InterruptsTable[intIdx] = this;

    switch(intIdx) {
    case 0:  attachInterrupt( intIdx, Button::interruptDown<0>,  FALLING ); break;
    case 1:  attachInterrupt( intIdx, Button::interruptDown<1>,  FALLING ); break;
    case 2:  attachInterrupt( intIdx, Button::interruptDown<2>,  FALLING ); break;
    case 3:  attachInterrupt( intIdx, Button::interruptDown<3>,  FALLING ); break;
    case 4:  attachInterrupt( intIdx, Button::interruptDown<4>,  FALLING ); break;
    case 5:  attachInterrupt( intIdx, Button::interruptDown<5>,  FALLING ); break;
    case 6:  attachInterrupt( intIdx, Button::interruptDown<6>,  FALLING ); break;
    case 7:  attachInterrupt( intIdx, Button::interruptDown<7>,  FALLING ); break;
    case 8:  attachInterrupt( intIdx, Button::interruptDown<8>,  FALLING ); break;
    case 9:  attachInterrupt( intIdx, Button::interruptDown<9>,  FALLING ); break;

    case 10: attachInterrupt( intIdx, Button::interruptDown<10>, FALLING );  break;
    case 11: attachInterrupt( intIdx, Button::interruptDown<11>, FALLING );  break;
    case 12: attachInterrupt( intIdx, Button::interruptDown<12>, FALLING );  break;
    case 13: attachInterrupt( intIdx, Button::interruptDown<13>, FALLING );  break;
    case 14: attachInterrupt( intIdx, Button::interruptDown<14>, FALLING );  break;
    case 15: attachInterrupt( intIdx, Button::interruptDown<15>, FALLING );  break;
    case 16: attachInterrupt( intIdx, Button::interruptDown<16>, FALLING );  break;
    case 17: attachInterrupt( intIdx, Button::interruptDown<17>, FALLING );  break;
    case 18: attachInterrupt( intIdx, Button::interruptDown<18>, FALLING );  break;
    case 19: attachInterrupt( intIdx, Button::interruptDown<19>, FALLING );  break;

    case 20: attachInterrupt( intIdx, Button::interruptDown<20>, FALLING );  break;
    case 21: attachInterrupt( intIdx, Button::interruptDown<21>, FALLING );  break;
    case 22: attachInterrupt( intIdx, Button::interruptDown<22>, FALLING );  break;
    case 23: attachInterrupt( intIdx, Button::interruptDown<23>, FALLING );  break;
    case 24: attachInterrupt( intIdx, Button::interruptDown<24>, FALLING );  break;
    case 25: attachInterrupt( intIdx, Button::interruptDown<25>, FALLING );  break;
    case 26: attachInterrupt( intIdx, Button::interruptDown<26>, FALLING );  break;
    case 27: attachInterrupt( intIdx, Button::interruptDown<27>, FALLING );  break;
    case 28: attachInterrupt( intIdx, Button::interruptDown<28>, FALLING );  break;
    case 29: attachInterrupt( intIdx, Button::interruptDown<29>, FALLING );  break;

    case 30: attachInterrupt( intIdx, Button::interruptDown<30>, FALLING );  break;
    case 31: attachInterrupt( intIdx, Button::interruptDown<31>, FALLING );  break;
    }
}

bool Button::update()
{
    if( !mIsEvent ) {
        return false;
    }

    if( mEventCallback ) {
        mEventCallback(mIsDown, interval(), mCollar);
    }

    if( mIsDown && !isCurrentlyDown() ) {
        onUp();
    }

    mIsEvent = false;

    return true;
}

bool Button::isCurrentlyDown()
{
    return digitalRead(mPin) == LOW;
}

int64_t Button::interval()
{
    return millis() - mMsecEvent;
}
