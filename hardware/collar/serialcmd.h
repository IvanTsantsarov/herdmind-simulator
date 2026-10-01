#ifndef SERIALCMD_H
#define SERIALCMD_H

#include "../arduino.h"

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
            HELP,
            RESTART,
            INFO,
            EUI,
            GPS,    // Get current position
            SAT,    // Get number of GPS sattelites
            BAT,
            RSSI,   // Get Received Signal Strength Indicator
            SNR,    // Get SNR (Signal-to-Noise Ratio
            SS,     // Get Signal Strength
            NKEY,   // Set/Get network key
            AKEY,   // Set/Get application key
            ADDR,    // Set/Get address
            FLASH,    // Store in memory
            RESTORE   // Restore from memory

        };


        inline Type type(){ return mT;}
        const char* typeStr();
        inline bool isNone(){ return Type::NONE == mT; }

        inline void clear(){ mT = Type::NONE; }
        bool parse(const char* buffer, int bufferLen);
    private:
        Type mT = Type::NONE;
    };

    Cmd mCmd;


    void update();
    void clear();

#ifdef ONPC
    inline void writeToSerial(const char* str) { Serial.writeIn(str); }
    inline QByteArray readFromSerial() { return Serial.readOut(); }
#endif
};

#endif // SERIALCMD_H
