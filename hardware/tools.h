#ifndef TOOLS_H
#define TOOLS_H

#include <cassert>
#include <cstdint>
#include <QString>

class DialogCollarSim;
class Animal;

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char byte;
#define FLOAT float

class Tools
{

public:

    Tools();
    static int16_t f2i16(float v, float scale);
    static float rnd(float minv, float maxv);
};


#define U8G2_R0 0

// Font
#define u8g2_font_spleen8x16_mf 0


#define SCREEN_COL_LIGHT QColor(0, 240, 255)
#define SCREEN_COL_DARK QColor(10, 10, 10)

class ScreenSim {
public:
    static const uint16_t mW = 128;
    static const uint16_t mH = 64;
    static const uint16_t mBytesCount = mW * mH;
    uint8_t mBuffer[mBytesCount];

private:
    uint8_t mPixelColor = 0x1; // 0 - clear, 1 - pixel, 2 - invert

    static const uint16_t mFontCX = 8;      // character width in pixels
    static const uint16_t mFontCY = 16;     // character height in pixels
    static const uint16_t mFontCountX = 16; // font columns count
    static const uint16_t mFontCountY = 16; // font rows count
    static const uint16_t mFontBorderX = 1; // width distance in pixels between chars
    static const uint16_t mFontBorderY = 1; // hight distance in pixels between chars

    static uint8_t mFont[mFontCX * mFontCY][mFontCY][mFontCX];

    DialogCollarSim* mDlg = nullptr;
    Animal * mAnimal = nullptr;

public:
    ScreenSim(int r0, int sda, int scl, int rst);

    void simulationInit( DialogCollarSim* dlgDevice, Animal* a);

    void begin(){}
    void clearBuffer() {
        for( auto i = 0; i < mBytesCount; i ++) {
            mBuffer[i] = 0;
        }
    };

    inline void setDrawColor(uint8_t color_index) { mPixelColor = color_index; }

    inline void drawPixel(int x, int y) {
        assert(x >= 0 && x < mW);
        assert(y >= 0 && y < mH);
        switch( mPixelColor ) {
        case 0: mBuffer[ y * mW + x ] = 0; break;
        case 1: mBuffer[ y * mW + x ] = 1; break;
        case 2: mBuffer[ y * mW + x ] = !mBuffer[ y * mW + x ]; break;
        }
    }

    inline uint8_t pixel(int x, int y) {
        assert(x >= 0 && x < mW);
        assert(y >= 0 && y < mH);
        return mBuffer[ y * mW + x ];
    }

    void drawHLine(int x, int y, int w);
    void drawVLine(int x, int y, int h);

    void setFont(int fontId);
    void drawStr(int x, int y, const char* str);
    void drawFrame(int x, int y, int cx, int cy);
    void sendBuffer();

};


void delay(int millis);


enum StringNumberType
{
    DEC = 1,
    HEX = 2
};

#define FLOAT_PRECISION 5
#define SIM_STRING_CONSTRUCTOR_NUMBER(__type__) String( __type__ value, StringNumberType type = DEC ) { mString = QString("%1").arg(value, 0, baseFromNumberType(type)); }


class String
{
    QString mString;
    QByteArray mLocalStr;

public:
    String(){}
    String(const char *cstr) : mString(cstr) {}

    String(const String& str) : mString(str.mString) {}
    String(const QString& str) : mString(str) {}
    String(char ch) {mString.append(ch);}
    SIM_STRING_CONSTRUCTOR_NUMBER(byte)
    SIM_STRING_CONSTRUCTOR_NUMBER(long)
    SIM_STRING_CONSTRUCTOR_NUMBER(ulong)
    SIM_STRING_CONSTRUCTOR_NUMBER(int)
    SIM_STRING_CONSTRUCTOR_NUMBER(uint)
    String(FLOAT value, uint precision = FLOAT_PRECISION) { mString = QString( "%1").arg(value, 0, 'g', precision); }
    void reserve(uint ){}

    bool equals(const String& str, bool isCaseSensitive = true);
    inline char operator [](uint index) { return mString[index].toLatin1(); }

    String operator+(const String& str) const{ return String(mString + str.mString); }
    void operator +=(const String& str){ mString += str.mString; }
    bool operator ==(const String& str) const{ return mString == str.mString; }
    String operator +(const char str[]) const{ return String(mString + String(str).mString); }

    int toInt() const;
    FLOAT toFloat() const;
    QString toQString() const{ return mString; }

    int length() const { return mString.length(); }

    void toUpperCase();
    void replace(char what, char with);

    static int baseFromNumberType(StringNumberType type);
    char* data()
    {
        mLocalStr = mString.toLocal8Bit();
        mLocalStr.append(1, 0);
        return mLocalStr.data();
    }

    const char* c_str() { return data(); }

    String substring(uint left, uint right) const;
    int indexOf(char ch) const;
    void trim(void);
};

String operator+(const char str[], const String& strObj);


#endif // TOOLS_H
