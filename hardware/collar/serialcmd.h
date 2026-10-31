#ifndef SERIALCMD_H
#define SERIALCMD_H

#include "../arduino.h"

#define SERIAL_CMD_BEGIN 'S'
#define SERIAL_CMD_PARAMS_DM '|'
#define SERIAL_CMD_COMMA ','
#define SERIAL_CMD_TAIL ':'
#define SERIAL_CMD_EQUAL '='


class Collar;

class SerialCmd
{
    char* mCommand = nullptr;
    int mCommandLen = 0;
    Collar* mCollar = nullptr;
    char* mArgument = nullptr;
    int mArgumentLen = 0;
    bool mIsArgument = false;
    const char* mErrStr = nullptr;
    String mStrChanged;

    void parse();
    bool execute();
    inline bool hasArgumentString(){ return 0 != mArgument[0]; }

    void skipAvailable();
public:

    SerialCmd(Collar *c);
    ~SerialCmd();
    struct Cmd {

        enum struct Type {
            NONE = 0,
            DBG,
            RSP,    // Responce - send back an aswer from action
            SIGL,   // Signal light
            SIGS,   // Signal sound
            SIGE,   // Signal electicity
            LED,    // led on/off
            HELP,
            REBOOT,
            INFO,
            VER,    // Give software version
            EUI,
            GPS,    // Get current position
            SAT,    // Get number of GPS sattelites
            BAT,
            RSSI,   // Get Received Signal Strength Indicator
            SNR,    // Get SNR (Signal-to-Noise Ratio
            SS,     // Get Signal Strength
            NAME,   // Get/Set animal name
            SEX,    // m or f
            NKEY,   // Set/Get network key
            AKEY,   // Set/Get application key
            ADDR,    // Set/Get address
            FPORT,  // Set/Get FPort
            FLASH,    // Store in memory
            RESTORE,   // Restore from memory
            LAST
        };

        Cmd(){ mT = Type::NONE; }
        Cmd(Type t){ mT = t; }

        inline Type type(){ return mT;}
        const char* typeStr();
        static const char* type2StrStatic(Type t);
        static Type str2TypeStatic(const char* str);
        inline bool isNone(){ return Type::NONE == mT; }
        inline operator int(){ return static_cast<int>(mT);}

        inline void clear(){ mT = Type::NONE; }
        bool parse(const char* buffer, int bufferLen);
    private:
        Type mT = Type::NONE;
    };

    Cmd mCmd;
    static const char* mCommands[(int)SerialCmd::Cmd::Type::LAST];
    static void initCommands();


    void setup();
    void update();
    void clear();

#ifdef ONPC
    inline void writeToSerial(const char* str) { Serial.writeIn(str); }
    inline QByteArray readFromSerial() { return Serial.readOut(); }
#endif

    void sendCmd(SerialCmd::Cmd::Type t, const String &str = String());
    void sendResponce(const String &rsp) { sendCmd(Cmd::Type::RSP, rsp); }
};

#endif // SERIALCMD_H
