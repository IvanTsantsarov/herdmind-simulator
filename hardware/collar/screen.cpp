#ifdef SIMULATION
    #include "animal.h"
#else
#endif


#include "screen.h"
#include "defines.h"
#include "collar.h"
#include "res.h"

Screen::Screen(const Collar* c) :
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
    // Initialize the mLib library and turn on the display
    mLib.begin();
}

void Screen::init() {
    // Clear the internal buffer
    mLib.clearBuffer();

    // Set a clean, readable text font (mLib has hundreds of choices)
    mLib.setFont(u8g2_font_spleen8x16_mf);

    mLib.drawFrame(0, 0, SCREEN_CX, SCREEN_CY);

    drawArray(2, 2, 44, 44, vector_mono_44x44);

    // Draw static strings (X position, Y position, String)
    // mLib.drawStr(2, 2 + 44 + FONT_CY, "Herdmind");

#ifdef SIMULATION
    // TODO: send animal name to the collar
    // mLib.drawStr(2, 2 + 44 + FONT_CY, mCollar->animal()->name().toStdString().c_str());
#endif

    String v("v");
    v += COLLAR_VERSION;
    mLib.drawStr(2, 2+ 44 + 2*FONT_CY, v.data());

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
