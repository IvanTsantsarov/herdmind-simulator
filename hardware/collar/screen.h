#ifndef SCREEN_H
#define SCREEN_H


#ifdef SIMULATION
    #include "../tools.h"
    #define ScreenLib ScreenSim
#else
    #include <Arduino.h>
    #include "drivers/u8g2/U8g2lib.h"
    #define ScreenLib U8G2_SSD1306_128X64_NONAME_F_SW_I2C
#endif


// Heltec V4 Onboard OLED PIN definitions
#define OLED_SDA   17
#define OLED_SCL   18
#define OLED_RST   21

class Screen
{
    ScreenLib mLib;
public:
    Screen();
    void setup();
    void test();
#ifdef SIMULATION
    ScreenLib& lib(){ return mLib;}
#endif
};

#endif // SCREEN_H
