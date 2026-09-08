#include "screen.h"

Screen::Screen() :
    // Initialize the SSD1315 using the standard SSD1306 Full Frame Buffer constructor over SW I2C
    mLib(U8G2_R0, OLED_SCL, OLED_SDA, OLED_RST)
{

}

void Screen::setup() {
    // Initialize the mLib library and turn on the display
    mLib.begin();
}

void Screen::test() {
    // Clear the internal buffer
    mLib.clearBuffer();

    // Set a clean, readable text font (mLib has hundreds of choices)
    mLib.setFont(u8g2_font_spleen8x16_mf);

    // Draw static strings (X position, Y position, String)
    mLib.drawStr(0, 20, "Heltec WiFi V4");
    mLib.drawStr(0, 40, "OLED Initialized!");

    // Draw a visual frame border around the 128x64 display
    mLib.drawFrame(0, 0, 128, 64);

    // Push the buffer contents to the physical screen hardware
    mLib.sendBuffer();

    delay(2000);
}
