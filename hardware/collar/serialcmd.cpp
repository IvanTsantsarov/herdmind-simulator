#include "serialcmd.h"

#include "collar.h"
#include "led.h"
#include "defines.h"

#define SERIAL_TX_BUFFER_SIZE 1024
#define SERIAL_ARGUMENT_SIZE 32

#define SERIAL_CMD_IS_SPACE(__char__) (__char__ == ' ')
#define SERIAL_CMD_END(__char__) (__char__ == '\r' || __char__ == '\n')

#define SERIAL_CMD_STRING_CHANGED "->"

const char* SerialCmd::mCommands[] = { "" };

void SerialCmd::initCommands() {
    if( mCommands[0][0] ) {
        return;
    }

    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::NONE] = "none";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::DBG] = "dbg";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::STATS] = "stats";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SLEEP] = "sleep";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::INT] = "int";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGL] = "sigl";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGS] = "sigs";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGE] = "sige";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::LED] = "led";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::HELP] = "help";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::REBOOT] = "reboot";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::RTM] = "rtm";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::TM] = "tm";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::INFO] = "info";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::EUI] = "eui";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::GPS] = "gps";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::BAT] = "bat";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::RADIO] = "radio";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::NKEY] = "nkey";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::AKEY] = "akey";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::ADDR] = "addr";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::FLASH] = "flash";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::RESTORE] = "restore";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::NAME] = "name";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SEX] = "sex";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::VER] = "ver";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::RSP] = "rsp";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::FPORT] = "fport";
}


SerialCmd::SerialCmd(Collar* c)
{
    SerialCmd::initCommands();

    mCollar = c;

    // Create the buffer
    mCommand = new char[SERIAL_CMD_SIZE + 1];
    mArgument = new char[SERIAL_ARGUMENT_SIZE + 1];
    clear();
}

SerialCmd::~SerialCmd()
{
    if( mCommand) {
        delete [] mCommand;
        mCommand = nullptr;
        mCommandLen = 0;
    }

    if( mArgument ) {
        delete [] mArgument;
        mArgument = nullptr;
        mArgumentLen = 0;
    }
}

void SerialCmd::setup()
{
    // allocate TX buffer for non-blocking sends
    Serial.setTxBufferSize(SERIAL_TX_BUFFER_SIZE);

    Serial.begin(SERIAL_BAUDRATE);

    /////////////////////////////////////////////////
    // Do not block here, because if it's disconected
    // from USB it will never start !
    // while(!Serial);

    delay(100);
    DBG("Print \"help\" for more info...");
}

void SerialCmd::clear()
{
    mCmd.clear();
    mCommand[0] = 0;
    mCommandLen = 0;
    mArgument[0] = 0;
    mArgumentLen = 0;
    mIsArgument = false;

}

void SerialCmd::printCmd(Cmd::Type t, const String& str)
{
    Cmd cmd(t);
    String r = String(SERIAL_CMD_BEGIN) + Cmd::type2StrStatic(t) + SERIAL_CMD_TAIL + (str.length() ? str : "");
    Serial.println(r);
    Serial.flush();
}


#define SERIAL_ERROR_RETURN(__err__)  printCmd(Cmd::Type::RSP, String(mCommand) + String("=>Error:") + (__err__)); clear(); skipAvailable(); return;

void SerialCmd::update()
{
    if( !mCommand ||!mCollar) {
        return;
    }

    while( Serial.available() ) {

        // Read and tap the buffer
        char ch = Serial.read();

        if( SERIAL_CMD_IS_SPACE(ch)) {
            if( mIsArgument ) {
                SERIAL_ERROR_RETURN("Second delimeter!");
            }

            mIsArgument = true;
            continue;
        }

        if( SERIAL_CMD_END(ch) ) {
            if( !mCmd.parse(mCommand, mCommandLen) ) {
                SERIAL_ERROR_RETURN("Unknown command!");
            }

            if( mIsArgument && !hasArgumentString() ) {
                SERIAL_ERROR_RETURN("Missing argument!");
            }

            execute();
            clear();
            return;
        }else {
            if( mIsArgument ) {

                if( mArgumentLen >= SERIAL_ARGUMENT_SIZE ) {
                    SERIAL_ERROR_RETURN("Argument too long!");
                }

                mArgument[mArgumentLen++] = ch;
                mArgument[mArgumentLen] = 0;

            }else {
                if( mCommandLen >= SERIAL_CMD_SIZE ) {
                    SERIAL_ERROR_RETURN("Command too long!");
                }
                mCommand[mCommandLen++] = ch;
                mCommand[mCommandLen] = 0;
            }
        }
    }
}


const char *SerialCmd::Cmd::typeStr()
{
    return SerialCmd::Cmd::type2StrStatic(this->mT);
}

const char* SerialCmd::Cmd::type2StrStatic(Type t)
{
    assert(t < SerialCmd::Cmd::Type::LAST);
    return mCommands[(int)t];
}

SerialCmd::Cmd::Type SerialCmd::Cmd::str2TypeStatic(const char* str)
{
    for( auto t = Type::NONE; t < Type::LAST; t = Type((int)t + 1)) {
        if( !strcmp(type2StrStatic(t), str) ) {
            return t;
        }
    }

    return Type::NONE;
}

bool SerialCmd::Cmd::parse(const char *buffer, int bufferLen)
 {
    int len = 0;
    char ch = buffer[len];

    // make it lowercase
    if( ch >= 'A' && ch <= 'Z') {
        ch += ('a' - 'A');
    }

    while( ch && len < bufferLen) {
        if( ch != buffer[len]){
            return false;
        }
        ch = buffer[++len];
    }

    mT = Cmd::str2TypeStatic(buffer);
    return Cmd::Type::NONE != mT;
}



bool SerialCmd::execute()
{
    auto printChanged = [&]() {
        printResponce(String(SERIAL_CMD_STRING_CHANGED) + mArgument);
    };

    auto printCommand = [&](const String& str) {
        Serial.println(String(SERIAL_CMD_BEGIN) + Cmd::type2StrStatic(mCmd.type())  + SERIAL_CMD_TAIL + str);
    };

    switch( mCmd.type() ) {
    case Cmd::Type::NONE:
        return false;
        break;
    case Cmd::Type::DBG:
        printResponce(mCollar->toggleDbg() ? "Debug On" : "Debug Off");
        break;
    case Cmd::Type::STATS:
        printResponce(mCollar->toggleStats() ? "Stats On" : "Stats Off");
        break;
    case Cmd::Type::LED:
        printResponce(mCollar->led()->toggle() ? "LED On" : "LED Off");
        break;
    case Cmd::Type::REBOOT:
        printResponce("Rebooting...");
        mCollar->reboot();
        break;
    case Cmd::Type::INFO:
        printCommand( mCollar->animalName() + SERIAL_CMD_PARAMS_DM +
                       (mCollar->isMale() ? "m":"f") + SERIAL_CMD_PARAMS_DM +
                        mCollar->euiHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->akeyHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->nkeyHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->addrHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->fport() );
        break;

    case Cmd::Type::GPS: {
        printResponce( String("gps=") + mCollar->gpsStatus() + SERIAL_CMD_PARAMS_DM +
                        "lat=" + String(mCollar->gps().mLat, 10) + SERIAL_CMD_PARAMS_DM +
                        "lon=" + String(mCollar->gps().mLon, 10) + SERIAL_CMD_PARAMS_DM +
                        "sat=" + mCollar->satellites() );
    }
    break;


    case Cmd::Type::BAT:
        printCommand( mCollar->batteryInfo() );
        break;

    case Cmd::Type::RADIO:
        printCommand( String("RSSI=") + mCollar->rssi() + SERIAL_CMD_PARAMS_DM
                       + "SNR=" + mCollar->snr() + SERIAL_CMD_PARAMS_DM
                       + "SS=" + mCollar->signalStrength());
        break;


    case Cmd::Type::INT:
    {
        if( !mIsArgument ) {
            printCommand( String("Send/sleep inverval is: ") + mCollar->sendInterval() + " msec" );
            return true;
        }


        const uint64_t msec = strtoull(mArgument, NULL, 10);
        if( msec < 1000) {
            printError("Interval is less than a second.");
            return false;
        }
        mCollar->setSendInterval(msec);
        printResponce( String("Send/sleep interval set to:") + msec + "msec" );
        break;
    }

    case Cmd::Type::SLEEP:
    {
        if( mIsArgument ) {
            char argChar = mArgument[0];
            bool is = (argChar == 'y' || argChar == '1' ? true : false);
            mCollar->enableDeepSleep(is);
        }

        printResponce( String("Deep sleep is:") + (mCollar->isDeepSleep() ? "Enabled" : "Disabled") );
        break;
    }
    case Cmd::Type::RTM:
        // Time sync requect
        printCmd(Cmd::Type::RTM);
        mIsWaitingForTimeSync = true;
        break;
    case Cmd::Type::TM:
        mIsWaitingForTimeSync = false;

        if( mIsArgument ) {
            const uint64_t secsEpoch = strtoull(mArgument, NULL, 10);
            if( secsEpoch < 1767225600) { // epoch of 01.01.20026
                printError("Wrong time (before 01.01.2026)!");
                return false;
            }

            int64_t diff = (int64_t)mCollar->epoch() - (int64_t)secsEpoch;

            if( diff ) {
                mCollar->setEpoch(secsEpoch);
                printResponce( String("Time set to:") + mCollar->dateTime() +
                    (diff < 0 ? "\rSecs slower:" : "\rSecs faster:") + abs(diff) );
            }else {
                printResponce( "Time is the same." );
            }
        }else {
            printCommand(mCollar->dateTime());
        }
        break;

    case Cmd::Type::NKEY:

        if( mIsArgument ) {
            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                printError( "Wrong netkey length!" );
                return false;
            }
            mCollar->setNKeyHex(mArgument);
            printChanged();
        }else {
            printCommand(mCollar->nkeyHex());
        }
        break;

    case Cmd::Type::AKEY:
        if( mIsArgument ) {

            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                printError("Wrong AppKey lenght!");
                return false;
            }
            mCollar->setAKeyHex(mArgument);
            printChanged();
        }else {
            printCommand(mCollar->akeyHex());
        }
        break;

    case Cmd::Type::NAME:
        if( mIsArgument ) {
            if( mArgumentLen < 3) {
                printError("Wrong name lenght!");
                return false;
            }
            mCollar->setAnimalName(mArgument);
            printChanged();
        }else {
            printCommand(mCollar->animalName());
        }
        break;

    case Cmd::Type::SEX:
        if( mIsArgument ) {
            if( mArgumentLen != 1) {
                printError("Wrong sex lenght!");
                return false;
            }
            mCollar->setIsMale(strcmp(mArgument,"m") == 0 ? true : false);
            printChanged();
        }else {
            printCommand(mCollar->isMale() ? "m" : "f");
        }
        break;

    case Cmd::Type::EUI:
        if( mIsArgument ) {

            if( mArgumentLen != LORA_EUI_HEX_LEN) {
                printError("Wrong EUI lenght!");
                return false;
            }
            mCollar->setEuiHex(mArgument);
            printChanged();
        }else {
            printCommand(mCollar->euiHex());
        }
        break;

    case Cmd::Type::ADDR:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_ADDR_HEX_LEN) {
                printError("Wrong Address lenght!");
                return false;
            }
            mCollar->setAddrHex(mArgument);
            printChanged();
        }else {
            printCommand(mCollar->addrHex());
        }
        break;

    case Cmd::Type::FPORT:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_FPORT_HEX_LEN) {
                printError("Wrong FPort lenght!");
                return false;
            }
            mCollar->setFPortHex(mArgument);
            printChanged();
        }else {
            printCommand(String("0x") + mCollar->fportHex());
        }
        break;

    case Cmd::Type::FLASH:
        if( !mCollar->flash() ) {
            printError("Flashing.");
        }else {
            printResponce("Flashing done.");
        }
        break;
    case Cmd::Type::RESTORE:

        if( !mCollar->restore() ) {
            printError("Restoring.");
        }else {
            printResponce("Restoring done.");
        }
        break;

    case Cmd::Type::VER:
        printCommand(COLLAR_VERSION);
        break;

    case Cmd::Type::RSP:
        // not receiving - just sending. Here because of the warning
        break;
    case Cmd::Type::SIGL:
        printResponce("Executing lights...");
        mCollar->execSignal(Collar::SignalType::Light);
        break;
    case Cmd::Type::SIGS:
        printResponce("Executing sound...");
        mCollar->execSignal(Collar::SignalType::Sound);
        break;
    case Cmd::Type::SIGE:
        printResponce("Executing electric shock...");
        mCollar->execSignal(Collar::SignalType::Shock);
        break;
    case Cmd::Type::HELP:
        printCommand(
        "---------- HELP ----------\r"
        "stats: Toggle statistics\r"
        "info: Common info\r"
        "tm: Get and Set(Epoch in seconds) time\r"
        "sleep: Enable/Disable deep sleep (y/n and 0/1)\r"
        "int: Get/Set send/sleep interval in milliseconds\r"
         "--- Animal\r"
        "name: Set/Get animal's name\r"
        "sex: Set/Get gender: f or m\r"
         "--- LoraWAN\r"
        "eui: Set/Get EUI of the LoraWAN module\r"
        "nkey: Set/Get network key (only OTA supported)\r"
        "akey: Set/Get app key (only OTA supported)\r"
         "--- Signals\r"
        "sigl: execute signal Lights\r"
        "sigs: execute signal Sound\r"
        "sige: execute signal Electrick shock\r"
         "--- Hardware\r"
        "led: enable/disable board LED\r"
        "gps: status(e-emulated, y-hardware present, n-no hardware), current position, sattelites count\r"
        "bat: Battery status\r"
        "radio: Get LoRa RSSI, SNR and SS\r"
         "--- Program\r"
        "flash: Flash values into peristent memory\r"
        "restore: Restore values from peristent memory\r"
        "reboot: Restart the device\r"
         "--- Legend\r"
         "EUI: LoRa device unique number\r"
         "RSSI: Received Signal Strength Indicator\r"
         "SNR: Signal-to-Noise Ratio\r"
         "SS: Signal Strength\r"
        "--------------------------\r\n");
        break;

    default:
        printError( String("Wrong command:") + mCmd.typeStr() );
        return false;
    }

    return true;
}

void SerialCmd::skipAvailable()
{
    // Flush the old content
    while(Serial.available()) {
        Serial.read();
    }
}

// Static debug info flag
bool gIsDbg = true;

void gPrintDbg(const String &str) {
    if(gIsDbg)    {
        SerialCmd::printCmd(SerialCmd::Cmd::Type::DBG, str);
        Serial.flush();
    }
}

void gPrintDbg(const char* chars) {
    gPrintDbg(String(chars));
}
