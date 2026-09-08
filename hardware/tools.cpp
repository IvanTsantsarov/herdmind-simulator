#include <cmath>
// #include <algorithm>
#include <QThread>
#include <QVariant>
#include <QImage>
#include "tools.h"
#include "dialogcollarsim.h"


uint8_t ScreenSim::mFont[mFontCX * mFontCY][mFontCY][mFontCX];

Tools::Tools() {}


int16_t Tools::f2i16(float v, float scale) {
    long s = lroundf(v * scale);
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    return (int16_t) s;
}


float Tools::rnd(float minv, float maxv)
{
    return minv + (maxv - minv) * (float)rand() * (1.0f / (float)RAND_MAX);
}


void delay(int millis) {
    QThread::msleep(millis);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ScreenSim
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScreenSim::ScreenSim(int r0, int sda, int scl, int rst) {
    (void) r0; (void) sda;(void) scl; (void) rst;
}

void ScreenSim::setFont(int fontId) {
    (void) fontId;

    QString fontFile;
    switch(fontId) {
    case u8g2_font_spleen8x16_mf: fontFile = "://font8x16.png"; break;
    }

    QImage img( fontFile );
    int colx = 0;
    int rowy = 0;

    for( auto iy = 0; iy < mFontCountY; iy ++ ) {
        for( auto ix = 0; ix < mFontCountX; ix ++ ) {
            for (auto py = 0; py < mFontCY; py ++) {
                for (auto px = 0; px < mFontCX; px ++) {
                    QRgb rgb = img.pixel(colx + px, rowy + py);
                    mFont[iy*mFontCountX + ix][py][px] = (qRed(rgb) + qGreen(rgb) + qBlue(rgb)) / 3;
                }
            }
            colx += (mFontCX + mFontBorderX);
        }
        rowy += (mFontCY + mFontBorderY);
    }
}


void ScreenSim::drawHLine(int x, int y, int w)
{
    for(auto i = 0; i < w; i ++) {
        drawPixel(x + i, y);
    }
}

void ScreenSim::drawVLine(int x, int y, int h)
{
    for(auto i = 0; i < h; i ++) {
        drawPixel(x, y + h);
    }
}

void ScreenSim::drawFrame(int x, int y, int cx, int cy)
{
    drawHLine(x, y, cx);
    drawHLine(x, y+cy, cx);
    drawVLine(x, y, cy);
    drawVLine(x+cx, y, cy);
}


void ScreenSim::drawStr(int x, int y, const char *str){
    const char* ptr = str;
    while(*ptr) {
        uint16_t index = *ptr;
        for( auto cy = 0; cy < mFontCY; cy ++ ){
            for( auto cx = 0; cx < mFontCX; cx ++ ) {
                if( mFont[index][cy][cx] )
                    drawPixel( x + cx, y + cy );
            }
        }

        ptr ++;
    }
}


void ScreenSim::sendBuffer()
{
    assert(mDlg);
    assert(mAnimal);
    mDlg->sendScreen(mAnimal);
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// String
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int String::baseFromNumberType(StringNumberType type)
{
    int base = 10;

    switch(type)
    {
    case DEC: base = 10; break;
    case HEX: base = 16; break;
    }

    return base;
}

String String::substring(uint left, uint right) const
{
    return String(mString.mid(left, right - left));
}

int String::indexOf(char ch) const
{
    return mString.indexOf(ch);
}

void String::trim()
{
    mString = mString.trimmed();
}



bool String::equals(const String &str, bool isCaseSensitive)
{
    return mString.compare(str.mString, isCaseSensitive ? Qt::CaseInsensitive : Qt::CaseInsensitive) == 0;
}


String operator+(const char str[], const String &strObj)
{
    String s(str);
    return s + strObj;
}


int String::toInt() const
{
    return QVariant(mString).toInt();
}

FLOAT String::toFloat() const
{
    return QVariant(mString).toFloat();
}

void String::toUpperCase()
{
    mString = mString.toUpper();
}

void String::replace(char what, char with)
{
    mString.replace( what, with);
}


