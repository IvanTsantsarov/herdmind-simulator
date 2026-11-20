#ifndef BUTTON_H
#define BUTTON_H

#include <cstdint>

typedef void (*onButtonClickCallback)(int64_t msecDown) ;

class Button
{
    int mPin = -1;
    bool mIsPressed = false;
    int64_t mMsecPressStart = 0;
    int64_t mMsecPressed = 0;
public:
    Button(int pinNumber);
    void setup(onButtonClickCallback clickCallback);
    inline bool isPressed() { return mIsPressed; }
    void update();
    inline int pin(){ return mPin; }
};

#endif // BUTTON_H
