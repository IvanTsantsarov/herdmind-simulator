#pragma once

#include "../arduino.h"

#ifdef ONPC
    #define ScreenLib ScreenSim
#else
    #include <Wire.h>
    #include <U8g2lib.h>
    #define ScreenLib U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#endif

class Collar;

// Heltec V4 Onboard OLED PIN definitions
#define OLED_SDA   17
#define OLED_SCL   18
#define OLED_RST   21

class Screen
{
    ScreenLib mLib;
    Collar* mCollar;
    bool mIsSleeping = false;
    int64_t mWakeMsec = 0;
public:
    struct CenterH {
        int x1, x2;
        inline int width() { return x2 - x1;}
    };

    Screen(Collar *c);
    void setup();
    void init();
    void clear();
    void drawArray(int x, int y, int cx, int cy, uint8_t* pixels);
    void drawText(int x, int y, const String &str);
    void drawText(int x, int y, const char *str);
    void drawTextTable(int col, int row, const String &str, int offsetCol = 0, int offsetRow = 0);
    CenterH drawTextTableCenterH(int row, const String &str, int offsetY = 0);
    void flush();

    void sleep();
    void wakeup();
    void off();
    inline bool isSleeping(){ return mIsSleeping; }
    inline int64_t msecWake(){ return millis() - mWakeMsec; }

    ScreenLib& lib(){ return mLib;}
};

