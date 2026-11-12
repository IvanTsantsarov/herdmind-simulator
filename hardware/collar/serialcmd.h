#ifndef SERIALCMD_H
#define SERIALCMD_H

#include "../arduino.h"

#define SERIAL_CMD_BEGIN 'S'
#define SERIAL_CMD_PARAMS_DM '|'
#define SERIAL_CMD_COMMA ','
#define SERIAL_CMD_TAIL ':'
#define SERIAL_CMD_EQUAL '='
#define SERIAL_CMD_ERROR "Error:"


class Collar;

class SerialCmd
{
    char* mCommand = nullptr;
    int mCommandLen = 0;
    Collar* mCollar = nullptr;
    char* mArgument = nullptr;
    int mArgumentLen = 0;
    bool mIsArgument = false;

    void parse();
    bool execute();
    inline bool hasArgumentString(){ return 0 != mArgument[0]; }

    void skipAvailable();
    bool mIsWaitingForTimeSync = false; // RTM command send and waiting to sync the time
public:

    SerialCmd(Collar *c);
    ~SerialCmd();
    struct Cmd {

        enum struct Type {
            NONE = 0,
            STATS,   // Statistics
            DBG,    // Debug info
            RSP,    // Responce - send back an aswer from action
            SIGL,   // Signal light
            SIGS,   // Signal sound
            SIGE,   // Signal electicity
            LED,    // led on/off
            HELP,
            REBOOT,
            SLEEP,  // Enable/Disable Deep sleep
            INT,    // Get set send (sleep) interval in seconds
            RTM,     // Request time (TM) message
            TM,     // Get/Set the time
            INFO,
            VER,    // Give software version
            EUI,
            GPS,    // Get gps hardware availability, current position and satelites count
            BAT,
            RADIO,   // Get LoRa RSSI( Received Signal Strength Indicator), SNR(Signal-to-Noise Ratio) and SS (Signal Strength)
            NAME,   // Get/Set animal name
            SEX,    // Get set animal gender: m or f
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

    inline bool isWaitingForTimeSync(){ return mIsWaitingForTimeSync; }
    void stopWaitingForTimeSync(){ mIsWaitingForTimeSync = false; }

#ifdef ONPC
    inline void writeToSerial(const char* str) { Serial.writeIn(str); }
    inline QByteArray readFromSerial() { return Serial.readOut(); }
#endif

    static void printCmd(SerialCmd::Cmd::Type t, const String &str = String());
    void printResponce(const String &str){ printCmd(Cmd::Type::RSP, str); }
    inline void printResponce(const char* str) { printResponce(String(str)); }
    void printError(const String &str) { printResponce( SERIAL_CMD_ERROR + str) ; }
    inline void printError(const char* str) { printError(String(str)); }
};

#endif // SERIALCMD_H
