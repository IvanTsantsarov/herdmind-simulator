#ifndef LED_H
#define LED_H

#include "../arduino.h"

#define LED_MAX_BUFFER 20

class Led
{
    int mPin = -1;
    uint32_t mMSec = 0;
    uint32_t mDuration = 0;
    uint32_t mDurationAfter = 0;
    bool mIsOn = false;
    bool mIsPhase2 = false;
    int mBufferLen = 0;
    bool mIsEnabled = true;

public:
    void setup(int pinNum);
    bool update();
    void on(uint32_t duration = 0, uint32_t durationAfter = 0);
    void updateOn(uint32_t duration = 0, uint32_t durationAfter = 0);
    void off();
    inline bool isOn(){ return mIsOn; }
    inline bool isEnabled(){ return mIsEnabled; }
    void enable(bool is);
    bool toggle();
};

#endif // LED_H
