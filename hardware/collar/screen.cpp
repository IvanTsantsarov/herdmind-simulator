#ifdef SIMULATION
    #include "animal.h"
#else
#endif


#include "screen.h"
#include "defines.h"
#include "collar.h"
#include "res.h"

#ifndef VEXT
#define VEXT 36
#endif

Screen::Screen(Collar* c) :
    // Initialize the SSD1315 using the standard SSD1306 Full Frame Buffer constructor over SW I2C
#ifdef SIMULATION
    mLib(U8G2_R0, OLED_SCL, OLED_SDA, OLED_RST, c->animal()),
#else
    mLib(U8G2_R0, OLED_SCL, OLED_SDA, OLED_RST),
#endif
    mCollar(c)
{

}

void Screen::setup() {
    pinMode(VEXT, OUTPUT);
    digitalWrite(VEXT, LOW); // Pull LOW to enable display power rail
    delay(100);

    // 2. Assign Heltec V4 physical pins to the hardware Wire instance
    Wire.setPins(OLED_SDA, OLED_SCL);
    Wire.begin();

    // Initialize the mLib library and turn on the display
    mLib.begin();
}

void Screen::init() {

    // Clear the internal buffer
    mLib.clearBuffer();

    // Set a clean, readable text font (mLib has hundreds of choices)
    mLib.setFont(u8g2_font_spleen6x12_mf);

    mLib.drawFrame(0, 0, SCREEN_CX, SCREEN_CY);

    drawArray(2, 2, 44, 44, vector_mono_44x44);

    // Draw static strings (X position, Y position, String)
//    String v("Herdmind ");
//    v += COLLAR_VERSION;
//    mLib.drawStr(2, 2 + 44 + FONT_CY, v.data());

#ifdef SIMULATION
    // TODO: send animal name to the collar
    mLib.drawStr(2, 2 + 44 + FONT_CY, mCollar->animalName().c_str());
#endif


    // Draw a visual frame border around the 128x64 display
    // mLib.drawFrame(0, 0, 128, 64);

    // Push the buffer contents to the physical screen hardware
    mLib.sendBuffer();
}

void Screen::drawArray(int x, int y, int cx, int cy, uint8_t *pixels)
{
    for( int iy = 0; iy < cy; iy ++) {
        for( int ix = 0; ix < cx; ix ++) {
            if( pixels[iy * cx + ix] ) {
                mLib.drawPixel(x + ix, y + iy);
            }
        }
    }
}
