#pragma once

#ifndef ONPC

#include "arduino.h"

class LoraDev
{
    uint8_t mEui[8];
    String mEuiHex;

    void getEsp32DevEUI();
public:
    const char* euiHex() { return mEuiHex.c_str(); }
    virtual void onSetup();

};
#endif