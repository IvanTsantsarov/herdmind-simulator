#include <cmath>
#include <QTimer>
// #include <algorithm>
#include <QThread>
#include <QVariant>
#include <QImage>
#include "tools.h"
#include "dialogcollarsim.h"



SoftwareSerial Serial;
SoftwareSerial Serial1;
SoftwareSerial Serial2;
SoftwareSerial Serial3;

SimPin Pins[SIM_PINS_COUNT];
WireSim Wire;

#define SIM_CHECK_PIN_RANGE(__pin__) Q_ASSERT( (__pin__ >= 0) && (__pin__ < SIM_PINS_COUNT) )



void attachInterrupt(byte interrupt, InterruptCallback func, INTERRUPT_TYPE type)
{
    byte pin = interrupt;

    switch(interrupt)
    {
    case 0: pin = 2; break;
    case 1: pin = 3; break;
    case 2: pin = 21; break;
    case 3: pin = 20; break;
    case 4: pin = 19; break;
    case 5: pin = 18; break;
    }

    SIM_CHECK_PIN_RANGE(pin);

    Pins[pin].mOnIntrerrupt = func;
    Pins[pin].mInterruptType = type;
}

int digitalPinToInterrupt(int pin)
{
    return pin;
}

void digitalWrite(byte pin, bool val)
{
    SIM_CHECK_PIN_RANGE(pin);

    SimPin& p = Pins[pin];
    if( val != p.mValue && p.mOnIntrerrupt )
    {
        p.mValue = val ? 1.0f : 0;
        p.mOnIntrerrupt();
    }else {
        p.mValue = val ? 1.0f : 0;
    }
}

bool digitalRead(byte pin)
{
    SIM_CHECK_PIN_RANGE(pin);
    return Pins[pin].mValue > 0.5f;
}

int analogRead(byte pin)
{
    SIM_CHECK_PIN_RANGE(pin);
    return Pins[pin].mValue * 1024;
}

void analogWrite(byte pin, int val)
{
    SIM_CHECK_PIN_RANGE(pin);

    SimPin& p = Pins[pin];
    if( val != p.mValue && p.mOnIntrerrupt )
    {
        p.mValue = (FLOAT)val / 1024.0f;
        p.mOnIntrerrupt();
    }else
        p.mValue = (FLOAT)val / 1024.0f;
}


void pinMode(byte pin, quint8 type)
{
    SIM_CHECK_PIN_RANGE(pin);
    Pins[pin].mType = type;
}



uint8_t ScreenSim::mFont[FONT_ROWS * FONT_COLS][FONT_CY][FONT_CX];
int ScreenSim::mLastFont = 0;

DialogCollarSim* ScreenSim::mDlg = nullptr;

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

ScreenSim::ScreenSim(int r0, int sda, int scl, int rst, const Animal *a)
    : mAnimal(a)
{
    (void) r0; (void) sda;(void) scl; (void) rst;
    clearBuffer();
}

void ScreenSim::setFont(int fontId) {

    if( fontId == mLastFont ) {
        return;
    }

    mLastFont = fontId;

    QString fontFile;
    switch(fontId) {
    case u8g2_font_spleen8x16_mf:
        fontFile = "://font8x16.png";
        break;
    case u8g2_font_spleen6x12_mf:
        fontFile = "://font6x12.png"; break;
        break;

    default:
        qCritical() << "Unknown font" << fontId;
        assert(0);
    }

    QImage img( fontFile );
    if( img.isNull() ) {
        qCritical() << "Error opening font:" << fontFile;
    }

    int rowy = 0;
    for( auto iy = 0; iy < FONT_ROWS; iy ++ ) {
        int colx = 0;
        for( auto ix = 0; ix < FONT_COLS; ix ++ ) {
            for (auto py = 0; py < FONT_CY; py ++) {
                for (auto px = 0; px < FONT_CX; px ++) {
                    QRgb rgb = img.pixel(colx + px, rowy + py);
                    uint8_t c = qGray(rgb) > 120  ?  255 : 0;
                    mFont[iy*FONT_COLS + ix][py][px] = c;
                }
            }
            colx += (FONT_CX + FONT_BX);
        }
        rowy += (FONT_CY + FONT_BY);
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
        drawPixel(x, y + i);
    }
}

void ScreenSim::drawFrame(int x, int y, int cx, int cy)
{
    drawHLine(x, y, cx);
    drawHLine(x, y+cy-1, cx);
    drawVLine(x, y, cy-1);
    drawVLine(x+cx-1, y, cy);
}


void ScreenSim::drawStr(int x, int y, char *str) {
    const char* ptr = str;
    bool isUtf8Byte = false;
    while(*ptr) {
        uint16_t index;

        if( mIsUtf8 )
        {
            isUtf8Byte = !isUtf8Byte;
            if( isUtf8Byte ) {
                ptr ++;
                continue;
            }
            index = *ptr - CYR_OFFSET;
        }else
        {
            index = *ptr - ASCII_OFFSET;
        }


        for( auto cy = 0; cy < FONT_CY; cy ++ ){
            for( auto cx = 0; cx < FONT_CX; cx ++ ) {
                if( mFont[index][cy][cx] ) {
                    drawPixel( x + cx, y + cy - FONT_CY );
                }
            }
        }
        ptr ++;
        x += FONT_CX + 1;
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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Serial port emulation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SERIAL_RETURN_IF_OFF(__operation__)     if( !mIsOn ) { qWarning() << "Serial" << __operation__ <<", but it's off."; return; }

uint SoftwareSerial::available()
{
    return mBuffer.length();
}

SoftwareSerial::SoftwareSerial()
{
    mPinRX =-1;
    mPinTX =-1;
}

SoftwareSerial::SoftwareSerial(quint8 rx, quint8 tx)
{
    mPinRX = rx;
    mPinTX = tx;
}

void SoftwareSerial::begin(uint baudRate, int a1, int a2, int a3)
{
    (void) baudRate;
    (void) a1;
    (void) a2;
    (void) a3;
    mIsOn = true;

}

char SoftwareSerial::read()
{
    char ch = mBuffer[0];
    mBuffer = mBuffer.right(mBuffer.length()-1);
    return ch;
}

void SoftwareSerial::write(char c)
{
    (void) c;
}

void SoftwareSerial::print(const String& str, bool isError)
{
    SERIAL_RETURN_IF_OFF("print");

    String ps = str;
    if( isError)
    {
        fprintf(stderr, "%s", ps.data());
        fflush(stderr);
    }
    else
    {
        fprintf(stdout, "%s", ps.data());
        fflush(stdout);
    }

}

void SoftwareSerial::flush()
{

}

void SoftwareSerial::println(const String& str, bool isError)
{
//    if( gMainWindow)
//        gMainWindow->printOutput(str.toQString());

    String strNew = str + String("\n");
    print(strNew, isError);
}

void SoftwareSerial::send(QByteArray &ba)
{
    SERIAL_RETURN_IF_OFF("send");
    mBuffer.append(ba);
}

void SoftwareSerial::end()
{
    mIsOn = false;
}



bool SoftwareSerial::operator !()
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TinyGPS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GPS_DELAY_SATELLITES_SIMULATION 2000

void TinyGPSPlus::Location::update(char *buffer)
{

    // mIsUpdated = true;
}

void TinyGPSPlus::clear()
{
    mBufferPos = 0;
    mBuffer[0] = 0;
}

TinyGPSPlus::TinyGPSPlus()
{
}

void TinyGPSPlus::setupSimulation()
{
    QTimer::singleShot(GPS_DELAY_SATELLITES_SIMULATION, this, &TinyGPSPlus::onReady);
}

void TinyGPSPlus::onReady()
{
    location.mIsValid = true;
    location.mIsUpdated = true;

    altitude.mMeters = 120;
    location.mLat = 42.140457;
    location.mLon = 24.758694;
    satellites.mValue = 4;
}

