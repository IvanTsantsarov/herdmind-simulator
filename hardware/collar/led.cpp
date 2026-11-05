#include "led.h"





void Led::setup(int pinNum)
{
    mPin = pinNum;
    pinMode(mPin, OUTPUT);
}

void Led::on(uint32_t duration, uint32_t durationAfter)
{
    if( !mIsEnabled ) {
        return;
    }

    mIsOn = true;
    mDuration = duration;
    mMSec = millis();
    mDurationAfter = duration + durationAfter;
    mIsPhase2 = false;
    hwOn();
}

void Led::updateOn(uint32_t duration, uint32_t durationAfter)
{
    if( update() ) on(duration, durationAfter);
}

void Led::off()
{
    if( !mIsEnabled ) {
        return;
    }

    mIsOn = false;
    hwOff();
}

void Led::enable(bool is)
{
    mIsEnabled = is;

    if( !mIsEnabled ) {
        off();
    }
}

bool Led::toggle()
{
    enable(!mIsEnabled);
    return mIsEnabled;
}

void Led::blink(uint32_t duration)
{
    hwOn();
    delay(duration);
    hwOff();
}



bool Led::update()
{
    if( !mDuration ) {
        return true;
    }

    uint32_t passed = millis() - mMSec;
    if( passed < mDuration ) {
        return false;
    }

    if( mIsPhase2 ) {
        if( passed > mDurationAfter )  {
            return true;
        }
        return false;
    }

    if( mPin < 0) {
        Serial.println("Error LED not setup!");
        return false;
    }

    off();
    mIsPhase2 = true;
    return false;
}
