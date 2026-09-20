#pragma once

#ifdef SIMULATION
    #include "../tools.h"
    #define ScreenLib ScreenSim
#else
    #include <Arduino.h>
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
public:
    Screen(Collar *c);
    void setup();
    void init();
    void clear();
    void drawArray(int x, int y, int cx, int cy, uint8_t* pixels);
    void drawText(int x, int y, String &str);
    void drawTextTable(int col, int row, String &str, int offsetCol = 0, int offsetRow = 0);
    void drawTextTableCenterH(int row, String &str, int offsetY = 0);
    void flush();

    ScreenLib& lib(){ return mLib;}
};

