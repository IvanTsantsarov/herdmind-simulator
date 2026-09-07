#include "screen.h"

Screen::Screen() {}

#ifdef SIMULATION

#else
#include <Arduino.h>
#include "drivers/u8g2/U8g2lib.h"
#endif

// Heltec V4 Onboard OLED PIN definitions
#define OLED_SDA   17
#define OLED_SCL   18
#define OLED_RST   21

// Initialize the SSD1315 using the standard SSD1306 Full Frame Buffer constructor over SW I2C
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, OLED_RST);

void Screen::setup() {
    // Initialize the U8g2 library and turn on the display
    u8g2.begin();
}

void Screen::test() {
    // Clear the internal buffer
    u8g2.clearBuffer();

    // Set a clean, readable text font (u8g2 has hundreds of choices)
    u8g2.setFont(u8g2_font_ncenB08_tr);

    // Draw static strings (X position, Y position, String)
    u8g2.drawStr(0, 20, "Heltec WiFi V4");
    u8g2.drawStr(0, 40, "OLED Initialized!");

    // Draw a visual frame border around the 128x64 display
    u8g2.drawFrame(0, 0, 128, 64);

    // Push the buffer contents to the physical screen hardware
    u8g2.sendBuffer();

    delay(2000);
}
