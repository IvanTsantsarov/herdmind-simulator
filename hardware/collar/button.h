#ifndef BUTTON_H
#define BUTTON_H

#include <cstdint>

#define BUTTON_INTERRUPTS_COUNT 32

class Collar;

class Button
{
public:

    typedef void (*EventCallback)(bool isDown, int64_t msec, Collar* c);


private:
    int mPin = -1;
    volatile bool mIsDown = false;
    volatile bool mIsEvent = false;
    volatile int64_t mMsecEvent = 0;
    Collar* mCollar = nullptr;
    EventCallback mEventCallback = nullptr;

    static void DownInterrupt();

    static Button* InterruptsTable[BUTTON_INTERRUPTS_COUNT];
    void onUp();
    void onDown();

    template <int N>
    static void interruptDown() {
    }

public:
    Button(int pinNumber);
    void setup(Button::EventCallback eventCallback = nullptr, Collar* c = nullptr);
    inline bool isDown() { return mIsDown; }
    int64_t interval();
    bool update(); // return true on event
    inline int pin(){ return mPin; }
    bool isCurrentlyDown();
};



#endif // BUTTON_H
