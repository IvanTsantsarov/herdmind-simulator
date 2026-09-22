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
    mLib(U8G2_R0, OLED_RST, OLED_SCL, OLED_SDA, c->animal()),
#else
    mLib(U8G2_R0, OLED_RST, OLED_SCL, OLED_SDA),
#endif
    mCollar(c)
{

}

void Screen::setup() {

    // VEXT powers the OLED on V4.
    pinMode(VEXT, OUTPUT);
    digitalWrite(VEXT, LOW); // Pull LOW to enable display power rail
    delay(100);

    // 2. Assign Heltec V4 physical pins to the hardware Wire instance
    //Wire.setPins(OLED_SDA, OLED_SCL);
    //Wire.begin();

    // Initialize the mLib library and turn on the display
    mLib.begin();

    // Set a clean, readable text font (mLib has hundreds of choices)
    mLib.setFont(u8g2_font_spleen6x12_mf);
}


void Screen::clear()
{
    mLib.clearBuffer();
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

void Screen::drawText(int x, int y, String &str)
{
    mLib.drawStr(x, y, str.c_str() );
}

void Screen::drawText(int x, int y, char* str)
{
    mLib.drawStr(x, y, str );
}


void Screen::drawTextTable(int col, int row, String &str, int offsetCol, int offsetRow)
{
    mLib.drawStr(col * FONT_CX + offsetCol, row * FONT_CY + offsetRow, str.c_str() );
}

Screen::CenterH Screen::drawTextTableCenterH(int row, String &str, int offsetY)
{
    CenterH c;
    int width = str.length() * FONT_CX;
    c.x1 = (SCREEN_CX - width) / 2;
    mLib.drawStr(c.x1, row * FONT_CY + offsetY, str.c_str() );
    c.x2 = c.x1 + width;
    return c;
}


void Screen::flush()
{
    mLib.sendBuffer();
}

void Screen::sleep()
{
    mLib.setPowerSave(1);
    mIsSleeping = true;
}

void Screen::wakeup()
{
    mLib.setPowerSave(0);
    mIsSleeping = false;
}
