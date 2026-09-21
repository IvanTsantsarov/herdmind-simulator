#ifndef BUTTON_H
#define BUTTON_H

class Button
{
    int mPin = -1;
    bool mIsPressed = false;
public:
    Button(int pinNumber);
    void setup();
    inline bool isPressed() { return mIsPressed; }
    void update();
    inline int pin(){ return mPin; }
};

#endif // BUTTON_H
