#include "led.h"

void Led::setup(int pinNum)
{
    mPin = pinNum;
    pinMode(mPin, OUTPUT);
}

void Led::on(uint32_t duration, uint32_t durationAfter)
{
    mIsOn = true;
    mDuration = duration;
    mMSec = millis();
    mDurationAfter = duration + durationAfter;
    mIsPhase2 = false;
    digitalWrite(mPin, HIGH);   // Turn the LED on
}

void Led::updateOn(uint32_t duration, uint32_t durationAfter)
{
    if( update() ) on(duration, durationAfter);
}

void Led::off()
{
    mIsOn = false;
    digitalWrite(mPin, LOW);   // Turn the LED off
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
