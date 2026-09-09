#ifndef HARDWARE_DEFINES_H
#define HARDWARE_DEFINES_H

// No more then 8 points, because it will be not transmitted to the lora device
// message will be 42 bytes
#define VIRTUAL_FENCE_MAX_POINTS 8

#define SCREEN_CX 128
#define SCREEN_CY 64

#define FONT_CX  6      // character width in pixels
#define FONT_CY 12      // character height in pixels
#define FONT_COLS  16   // fonts count in a row
#define FONT_ROWS  12   // rows of fonts count
#define FONT_BX  1      // width distance in pixels between chars
#define FONT_BY  1      // hight distance in pixels between chars
#define ASCII_OFFSET  32     // offset of alphabet from ascii table
#define CYR_OFFSET  32      // offset of cyrilic alphabet from ascii table




#endif // HARDWARE_DEFINES_H
