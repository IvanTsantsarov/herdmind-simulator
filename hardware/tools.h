#pragma once

#include <cstdlib>
#include <cassert>
#include <cstdint>
#include <QString>
#include <QElapsedTimer>
#include <QDateTime>

#include <QObject>

#include "defines.h"
#include "lora.h"

#define LOW 0
#define HIGH 1

#define INPUT 0
#define OUTPUT 1

#define SERIAL_8N1 0

#define INPUT_PULLUP 0

#define ESP_MAC_WIFI_STA 0

#define LORAWAN_APP_DATA_MAX_SIZE 64

inline void esp_read_mac(uint8_t mac[], int arg1) {
    (void) mac, (void) arg1;
};

class DialogCollarSim;
class Animal;

#define SIM_PINS_COUNT 100

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char byte;
#define FLOAT float

enum INTERRUPT_TYPE
{
    UP = 0,
    DOWN = 1,
    CHANGE = 2,
    FALLING = 3
};


QDateTime fromTimestamp(uint32_t secFrom2K);

typedef void (*InterruptCallback)();
void attachInterrupt(byte interrupt, InterruptCallback func, INTERRUPT_TYPE type);

void delay(int millis);

bool digitalRead(byte pin );
void digitalWrite(byte pin, bool val );

int analogRead(byte pin );
void analogWrite(byte pin, int val );

int analogReadMilliVolts(byte pin);

void pinMode(byte pin, byte type);
int digitalPinToInterrupt(int pin);

struct SimPin
{
    FLOAT mValue;
    byte mType;
    inline bool isInput(){return mType == INPUT;}
    inline bool isOutput(){return mType == OUTPUT;}

    INTERRUPT_TYPE mInterruptType;
    InterruptCallback mOnIntrerrupt = NULL;
};

extern SimPin Pins[SIM_PINS_COUNT];

class Tools
{
    QElapsedTimer mProgramTimer;
    DialogCollarSim* mCollarDlg = nullptr;
public:

    Tools();
    void setup();

    inline DialogCollarSim* collarDlg(){ return mCollarDlg; }

    inline uint32_t millis(){ return mProgramTimer.elapsed(); }
    static int16_t f2i16(float v, float scale);
    static float rnd(float minv, float maxv);
};

extern Tools gTools;

inline uint32_t millis() { return gTools.millis(); }


#define U8G2_R0 0

// Font
#define u8g2_font_spleen8x16_mf 1
#define u8g2_font_spleen6x12_mf 2

#define SCREEN_COL_DARK QColor(10, 10, 10)

#define SCREEN_COL_LIGHT QColor(245, 152, 66)
#define SCREEN_COL_SLEEPING QColor(145, 52, 0)

#define SCREEN_COL_MIRROR QColor(0, 240, 255)
#define SCREEN_COL_MIRROR_SLEEPING QColor(0, 40, 50)


class ScreenSim {
public:
    static const uint16_t mBytesCount = SCREEN_CX * SCREEN_CY;
    uint8_t mBuffer[mBytesCount];

private:
    uint8_t mPixelColor = 0x1; // 0 - clear, 1 - pixel, 2 - invert


    static uint8_t mFont[FONT_ROWS * FONT_COLS][FONT_CY][FONT_CX];
    static int mLastFont;

    const Animal* mAnimal;

    bool mIsUtf8 = false;
    bool mIsPowerSave = false;

public:
    ScreenSim(int r0, int sda, int scl, int rst, const Animal* a);

    inline void setPowerSave(bool is){ mIsPowerSave = is; }

    void begin(){}
    void clearBuffer() {
        for( auto i = 0; i < mBytesCount; i ++) {
            mBuffer[i] = 0;
        }
    };

    inline void setDrawColor(uint8_t color_index) { mPixelColor = color_index; }

    inline void drawPixel(int x, int y) {

        if(x < 0 || x >= SCREEN_CX) {
            return;
        }

        if(y < 0 || y >= SCREEN_CY) {
            return;
        }

        int index = y * SCREEN_CX + x;
        switch( mPixelColor ) {
        case 0: mBuffer[ index ] = 0; break;
        case 1: mBuffer[ index ] = 1; break;
        case 2: mBuffer[ index ] = !mBuffer[ index ]; break;
        }
    }

    inline uint8_t pixel(int x, int y) {
        assert(x >= 0 && x < SCREEN_CX);
        assert(y >= 0 && y < SCREEN_CY);
        return mBuffer[ y * SCREEN_CX + x ];
    }

    void drawHLine(int x, int y, int w);
    void drawVLine(int x, int y, int h);

    void enableUTF8Print(){ mIsUtf8 = true;}

    static void setFont(int fontId);
    void drawStr(int x, int y, const char *str);
    void drawFrame(int x, int y, int cx, int cy) ;
    void sendBuffer();

};



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


    char* c_str(){ return data(); }

    String substring(uint left, uint right) const;
    int indexOf(char ch) const;
    void trim(void);
};

String operator+(const char str[], const String& strObj);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Serial
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class SoftwareSerial
{
    byte mPinRX, mPinTX;

    QByteArray mBufferIn, mBufferOut;
    bool mIsOn = false;

public:
    SoftwareSerial();
    SoftwareSerial(byte rx, byte tx);
    void begin(uint baudRate, int a1 = 0, int a2 = 0, int a3 = 0);
    uint available();
    char read();
    void write(char c);
    void print (const String& str, bool isError = false);
    void println(const String &str, bool isError = false);
    void send(QByteArray& ba);
    void end();

    bool operator !();
    void flush();

    void writeIn(const char* str);
    QByteArray readOut();
    void setTxBufferSize(int size) {(void )size; }
};

extern SoftwareSerial Serial;
extern SoftwareSerial Serial1;
extern SoftwareSerial Serial2;
extern SoftwareSerial Serial3;

class WireSim {
public:
    WireSim(){}
    void setPins(int a, int b) {}
    void begin(){};

};


#define GPS_MAX_BUFFER_SIZE 100

class TinyGPSPlus : public QObject
{
    Q_OBJECT

    int mBufferPos = 0;
    char mBuffer[GPS_MAX_BUFFER_SIZE+1];
    inline char* buffer(){ return mBuffer; }
    void clear();


public:
    TinyGPSPlus();
    void setupSimulation();
    class Location {
        friend class TinyGPSPlus;
    protected:
        bool mIsUpdated = false;
        bool mIsValid = false;
        float mLat = 0.0f;
        float mLon = 0.0f;
    public:
        float lat(){ return mLat; }
        float lng(){ return mLon; }
        void update(char* buffer);
        bool isUpdated(){ return mIsUpdated; }
        bool isValid(){ return mIsValid; }
    };

    class Satellites {
        friend class TinyGPSPlus;
    protected:
        int mValue = 3;
    public:
        inline int value() { return mValue; }
    };

    class Altitude {
        friend class TinyGPSPlus;
    protected:
        int mMeters = 100.0f;
    public:
        inline int meters() { return mMeters; }
    };

    Location location;
    Satellites satellites;
    Altitude altitude;

    void encode(char c) {
        if( mBufferPos < GPS_MAX_BUFFER_SIZE) {
            mBuffer[mBufferPos++] = c;
            mBuffer[mBufferPos] = 0;
            location.update(mBuffer);
            if( location.isUpdated() ) {
                clear();
            }
        }
    }

    void inject(float lat, float lon, float alt, int sat);

private slots:
    void onReady();
};

extern WireSim Wire;


#define ADC_11db 0
inline void analogReadResolution(int res) {}
inline void analogSetAttenuation(int arg){}

int constrain(int val, int minval, int maxval);


class QSettings;

class Preferences
{
    QSettings* mSettings = nullptr;
public:
    Preferences();
    ~Preferences();
    void begin(const char *name, bool readOnly = false, const char *partition_label = NULL);
    void end();

    size_t putBytes(const char* key, const void* buf, size_t len);
    size_t getBytes(const char* key, void * buf, size_t maxLen);

};


class ClassSPI {
public:
    void begin(int sck, int miso, int mosi, int nss) { }
};

extern ClassSPI SPI;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Lora(WAN)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if USING_LORA == LORA_RADIOLIB

#define RADIOLIB_ERR_NONE 0
#define RADIOLIB_LORAWAN_NEW_SESSION -1118
#define RADIOLIB_LORAWAN_SESSION_RESTORED -1117

struct ConfigLoRa_t {
    int frequency;
};


class Module {
public:
    Module() {}
    Module(uint32_t cs, uint32_t irq, uint32_t rst, uint32_t gpio) { }
    int begin(ConfigLoRa_t& c) {return 0; }

};

class SX1262: public Module {
public:
    SX1262(Module* m){ }

};

extern const int EU868;


class LoRaWANNode {
public:
    LoRaWANNode(Module* radio, const int* standart) { }
    int beginABP(uint8_t addr, uint8_t* fNwkSIntKey, uint8_t* sNwkSIntKey, uint8_t* nwkSEncKey, uint8_t* appSKey) { return RADIOLIB_ERR_NONE; }
    int activateABP() {return RADIOLIB_LORAWAN_NEW_SESSION; }
    int setRx2Dr(int interval){ return RADIOLIB_ERR_NONE; }
    int setRxWindowDelayCorrection(int val){ return RADIOLIB_ERR_NONE; }

    int sendReceive(const uint8_t* dataUp, size_t lenUp, uint8_t fPort, bool isConfirmed){ return lenUp; }
    int sendReceive(const uint8_t* dataUp, size_t lenUp, uint8_t fPort,
                    const uint8_t* dataDown, size_t* lenDown, bool isConfirmed){ return lenUp; }
};

#elif USING_LORA == LORA_HELTEC

#define ACTIVE_REGION 0
typedef int LoRaMacRegion_t ;

#define CLASS_A 0
typedef int DeviceClass_t;

extern uint8_t appData[LORAWAN_APP_DATA_MAX_SIZE];
extern size_t appDataSize;

#define DEVICE_STATE_INIT 1
#define DEVICE_STATE_JOIN 2
#define DEVICE_STATE_SEND 3
#define DEVICE_STATE_CYCLE 4
#define DEVICE_STATE_SLEEP 5

extern int deviceState;

class HeltecLoraWAN {
public:
    void generateDeveuiByChipID(){};
    void init(int lClass, int lRegion){};
    void join(){};
    void send(){};
    void cycle(int msec){};
    void sleep(int lClass){};

};

extern HeltecLoraWAN LoRaWAN;

#define HELTEC_BOARD 1
#define SLOW_CLK_TPYE 2

class HeltecMCU {
public:
    void begin(int board, int clk){}
};

extern HeltecMCU Mcu;

#endif
