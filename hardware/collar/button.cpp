#include "button.h"

#include "../arduino.h"


Button* Button::InterruptsTable[BUTTON_INTERRUPTS_COUNT] = {0};

template<> void Button::interruptUp<0>() { Button::InterruptsTable[0]->onUp(); }
template<> void Button::interruptUp<1>() { Button::InterruptsTable[1]->onUp(); }
template<> void Button::interruptUp<2>() { Button::InterruptsTable[2]->onUp(); }
template<> void Button::interruptUp<3>() { Button::InterruptsTable[3]->onUp(); }
template<> void Button::interruptUp<4>() { Button::InterruptsTable[4]->onUp(); }
template<> void Button::interruptUp<5>() { Button::InterruptsTable[5]->onUp(); }
template<> void Button::interruptUp<6>() { Button::InterruptsTable[6]->onUp(); }
template<> void Button::interruptUp<7>() { Button::InterruptsTable[7]->onUp(); }
template<> void Button::interruptUp<9>() { Button::InterruptsTable[9]->onUp(); }

template<> void Button::interruptUp<10>() { Button::InterruptsTable[10]->onUp(); }
template<> void Button::interruptUp<11>() { Button::InterruptsTable[11]->onUp(); }
template<> void Button::interruptUp<12>() { Button::InterruptsTable[12]->onUp(); }
template<> void Button::interruptUp<13>() { Button::InterruptsTable[13]->onUp(); }
template<> void Button::interruptUp<14>() { Button::InterruptsTable[14]->onUp(); }
template<> void Button::interruptUp<15>() { Button::InterruptsTable[15]->onUp(); }
template<> void Button::interruptUp<16>() { Button::InterruptsTable[16]->onUp(); }
template<> void Button::interruptUp<17>() { Button::InterruptsTable[17]->onUp(); }
template<> void Button::interruptUp<19>() { Button::InterruptsTable[19]->onUp(); }

template<> void Button::interruptUp<20>() { Button::InterruptsTable[20]->onUp(); }
template<> void Button::interruptUp<21>() { Button::InterruptsTable[21]->onUp(); }
template<> void Button::interruptUp<22>() { Button::InterruptsTable[22]->onUp(); }
template<> void Button::interruptUp<23>() { Button::InterruptsTable[23]->onUp(); }
template<> void Button::interruptUp<24>() { Button::InterruptsTable[24]->onUp(); }
template<> void Button::interruptUp<25>() { Button::InterruptsTable[25]->onUp(); }
template<> void Button::interruptUp<26>() { Button::InterruptsTable[26]->onUp(); }
template<> void Button::interruptUp<27>() { Button::InterruptsTable[27]->onUp(); }
template<> void Button::interruptUp<29>() { Button::InterruptsTable[29]->onUp(); }

template<> void Button::interruptUp<30>() { Button::InterruptsTable[30]->onUp(); }
template<> void Button::interruptUp<31>() { Button::InterruptsTable[31]->onUp(); }


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
    mIsEvent = true;
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
    pinMode(mPin, INPUT_PULLUP);

    mEventCallback = eventCallback;
    mCollar = c;

    int intIdx = digitalPinToInterrupt(mPin);

    assert(intIdx >= 0 && intIdx < BUTTON_INTERRUPTS_COUNT);

    Button::InterruptsTable[intIdx] = this;

    switch(intIdx) {
    case 0: attachInterrupt( intIdx, Button::interruptUp<0>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<0>, RISING ); break;
    case 1: attachInterrupt( intIdx, Button::interruptUp<1>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<1>, RISING ); break;
    case 2: attachInterrupt( intIdx, Button::interruptUp<2>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<2>, RISING ); break;
    case 3: attachInterrupt( intIdx, Button::interruptUp<3>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<3>, RISING ); break;
    case 4: attachInterrupt( intIdx, Button::interruptUp<4>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<4>, RISING ); break;
    case 5: attachInterrupt( intIdx, Button::interruptUp<5>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<5>, RISING ); break;
    case 6: attachInterrupt( intIdx, Button::interruptUp<6>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<6>, RISING ); break;
    case 7: attachInterrupt( intIdx, Button::interruptUp<7>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<7>, RISING ); break;
    case 8: attachInterrupt( intIdx, Button::interruptUp<8>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<8>, RISING ); break;
    case 9: attachInterrupt( intIdx, Button::interruptUp<9>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<9>, RISING ); break;

    case 10: attachInterrupt( intIdx, Button::interruptUp<10>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<10>, RISING ); break;
    case 11: attachInterrupt( intIdx, Button::interruptUp<11>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<11>, RISING ); break;
    case 12: attachInterrupt( intIdx, Button::interruptUp<12>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<12>, RISING ); break;
    case 13: attachInterrupt( intIdx, Button::interruptUp<13>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<13>, RISING ); break;
    case 14: attachInterrupt( intIdx, Button::interruptUp<14>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<14>, RISING ); break;
    case 15: attachInterrupt( intIdx, Button::interruptUp<15>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<15>, RISING ); break;
    case 16: attachInterrupt( intIdx, Button::interruptUp<16>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<16>, RISING ); break;
    case 17: attachInterrupt( intIdx, Button::interruptUp<17>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<17>, RISING ); break;
    case 18: attachInterrupt( intIdx, Button::interruptUp<18>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<18>, RISING ); break;
    case 19: attachInterrupt( intIdx, Button::interruptUp<19>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<19>, RISING ); break;

    case 20: attachInterrupt( intIdx, Button::interruptUp<20>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<20>, RISING ); break;
    case 21: attachInterrupt( intIdx, Button::interruptUp<21>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<21>, RISING ); break;
    case 22: attachInterrupt( intIdx, Button::interruptUp<22>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<22>, RISING ); break;
    case 23: attachInterrupt( intIdx, Button::interruptUp<23>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<23>, RISING ); break;
    case 24: attachInterrupt( intIdx, Button::interruptUp<24>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<24>, RISING ); break;
    case 25: attachInterrupt( intIdx, Button::interruptUp<25>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<25>, RISING ); break;
    case 26: attachInterrupt( intIdx, Button::interruptUp<26>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<26>, RISING ); break;
    case 27: attachInterrupt( intIdx, Button::interruptUp<27>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<27>, RISING ); break;
    case 28: attachInterrupt( intIdx, Button::interruptUp<28>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<28>, RISING ); break;
    case 29: attachInterrupt( intIdx, Button::interruptUp<29>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<29>, RISING ); break;

    case 30: attachInterrupt( intIdx, Button::interruptUp<30>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<30>, RISING ); break;
    case 31: attachInterrupt( intIdx, Button::interruptUp<31>, FALLING ); attachInterrupt( intIdx, Button::interruptDown<31>, RISING ); break;
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
