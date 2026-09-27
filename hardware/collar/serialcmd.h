#ifndef SERIALCMD_H
#define SERIALCMD_H

class Collar;

class SerialCmd
{
    char* mBuffer = nullptr;
    int mBufferLen = 0;
    Collar* mCollar = nullptr;

    void parse();
    void execute();
public:

    SerialCmd(Collar *c);
    ~SerialCmd();
    struct Cmd {

        enum struct Type {
            NONE = 0,
            RESTART,
            INFO,
            EUI,
            GPS,    // Current position
            SAT,    // Number of GPS sattelites
            BAT,
            RSSI,   // Received Signal Strength Indicator
            SNR,    // SNR (Signal-to-Noise Ratio
            SS      // Signal Strength
        };


        inline Type type(){ return mT;}
        inline bool isNone(){ return Type::NONE == mT; }

        inline void clear(){ mT = Type::NONE; }
        bool parse(const char* buffer, int bufferLen);
    private:
        Type mT = Type::NONE;
    };

    Cmd mCmd;


    void update();
    void clear();
};

#endif // SERIALCMD_H
