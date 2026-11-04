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
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGL] = "sigl";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGS] = "sigs";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGE] = "sige";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::LED] = "led";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::HELP] = "help";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::REBOOT] = "reboot";
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
    while(!Serial);
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

void SerialCmd::sendCmd(Cmd::Type t, const String& str)
{
    Cmd cmd(t);
    String r = String(SERIAL_CMD_BEGIN) + Cmd::type2StrStatic(t) + SERIAL_CMD_TAIL + (str.length() ? str : "");
    Serial.println(r);
}


#define SERIAL_ERROR_RETURN(__err__)  sendCmd(Cmd::Type::RSP, String("Error") + __err__); clear(); skipAvailable(); return;

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
        Serial.println(String(SERIAL_CMD_STRING_CHANGED) + mArgument) ;
    };

    auto printError = [&](const String& str) {
        Serial.println(String(SERIAL_CMD_ERROR) + str) ;
    };

    auto printHead = [&](bool isResponce = true) {
        String head = String(SERIAL_CMD_BEGIN)
                    + ( isResponce ? Cmd::type2StrStatic(Cmd::Type::RSP) : Cmd::type2StrStatic(mCmd.type()) )
                    + SERIAL_CMD_TAIL;

        Serial.print(head);
    };

    switch( mCmd.type() ) {
    case Cmd::Type::NONE:
        return false;
        break;
    case Cmd::Type::DBG:
        printHead();
        Serial.println(mCollar->toggleDbg() ? "Debug On" : "Debug Off");
        break;
    case Cmd::Type::STATS:
        printHead();
        Serial.println(mCollar->toggleStats() ? "Stats On" : "Stats Off");
        break;
    case Cmd::Type::LED:
        printHead();
        Serial.println(mCollar->led()->toggle() ? "LED On" : "LED Off");
        break;
    case Cmd::Type::REBOOT:
        printHead();
        Serial.println("Rebooting...");
        mCollar->reboot();
        break;
    case Cmd::Type::INFO:
        printHead(false);
        Serial.println( mCollar->animalName() + SERIAL_CMD_PARAMS_DM +
                       (mCollar->isMale() ? "m":"f") + SERIAL_CMD_PARAMS_DM +
                        mCollar->euiHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->akeyHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->nkeyHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->addrHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->fport() );
        break;

    case Cmd::Type::GPS: {
        printHead();
        Serial.println( String("gps=") + mCollar->gpsStatus() + SERIAL_CMD_PARAMS_DM +
                        "lat=" + String(mCollar->gps().mLat, 10) + SERIAL_CMD_PARAMS_DM +
                        "lon=" + String(mCollar->gps().mLon, 10) + SERIAL_CMD_PARAMS_DM +
                        "sat=" + mCollar->satellites() );
    }
    break;


    case Cmd::Type::BAT:
        printHead(false);
        Serial.println( mCollar->batteryInfo() );
        break;

    case Cmd::Type::RADIO:
        printHead(false);
        Serial.println( String("RSSI=") + mCollar->rssi() + SERIAL_CMD_PARAMS_DM
                       + "SNR=" + mCollar->snr() + SERIAL_CMD_PARAMS_DM
                       + "SS=" + mCollar->signalStrength());
        break;

    case Cmd::Type::NKEY:

        if( mIsArgument ) {
            printHead();
            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                printError( "Wrong netkey length!" );
                return false;
            }
            mCollar->setNKeyHex(mArgument);
            printChanged();
        }else {
            printHead(false);
            Serial.println(mCollar->nkeyHex());
        }
        break;

    case Cmd::Type::AKEY:
        if( mIsArgument ) {
            printHead();
            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                printError("Wrong AppKey lenght!");
                return false;
            }
            mCollar->setAKeyHex(mArgument);
            printChanged();
        }else {
            printHead(false);
            Serial.println(mCollar->akeyHex());
        }
        break;

    case Cmd::Type::NAME:
        printHead();
        if( mIsArgument ) {
            if( mArgumentLen < 3) {
                printError("Wrong name lenght!");
                return false;
            }
            mCollar->setAnimalName(mArgument);
            printChanged();
        }else {
            printHead(false);
            Serial.println(mCollar->animalName());
        }
        break;

    case Cmd::Type::SEX:
        if( mIsArgument ) {
            printHead();
            if( mArgumentLen != 1) {
                printError("Wrong sex lenght!");
                return false;
            }
            mCollar->setIsMale(strcmp(mArgument,"m") == 0 ? true : false);
            printChanged();
        }else {
            printHead(false);
            Serial.println(mCollar->isMale() ? "m" : "f");
        }
        break;

    case Cmd::Type::EUI:
        if( mIsArgument ) {
            printHead();
            if( mArgumentLen != LORA_EUI_HEX_LEN) {
                printError("Wrong EUI lenght!");
                return false;
            }
            mCollar->setEuiHex(mArgument);
            printChanged();
        }else {
            printHead(false);
            Serial.println(mCollar->euiHex());
        }
        break;

    case Cmd::Type::ADDR:
        if( mIsArgument ) {
            printHead();
            if( mArgumentLen != LORA_ADDR_HEX_LEN) {
                printError("Wrong Address lenght!");
                return false;
            }
            mCollar->setAddrHex(mArgument);
            printChanged();
        }else {
            printHead(false);
            Serial.println(mCollar->addrHex());
        }
        break;

    case Cmd::Type::FPORT:
        if( mIsArgument ) {
            printHead();
            if( mArgumentLen != LORA_FPORT_HEX_LEN) {
                printError("Wrong FPort lenght!");
                return false;
            }
            mCollar->setFPortHex(mArgument);
            printChanged();
        }else {
            printHead(false);
            Serial.println(String("0x") + mCollar->fportHex());
        }
        break;

    case Cmd::Type::FLASH:
        printHead();
        if( !mCollar->flash() ) {
            printError("Flashing.");
        }else {
            Serial.println("Flashing done.");
        }
        break;
    case Cmd::Type::RESTORE:
        printHead();
        if( !mCollar->restore() ) {
            printError("Restoring.");
        }else {
            Serial.println("Restoring done.");
        }
        break;

    case Cmd::Type::VER:
        printHead(false);
        Serial.println(COLLAR_VERSION);
        break;
    case Cmd::Type::RSP:
        // not receiving - just sending. Here because of the warning
        break;
    case Cmd::Type::SIGL:
        printHead();
        Serial.println("Executing lights...");
        mCollar->execSignal(Collar::SignalType::Light);
        break;
    case Cmd::Type::SIGS:
        printHead();
        Serial.println("Executing sound...");
        mCollar->execSignal(Collar::SignalType::Sound);
        break;
    case Cmd::Type::SIGE:
        printHead();
        Serial.println("Executing electric shock...");
        mCollar->execSignal(Collar::SignalType::Shock);
        break;
    case Cmd::Type::HELP:
        printHead(false);
        Serial.print("---------- HELP ----------\r");
        Serial.print("stats: Toggle statistics\r");
        Serial.print("info: Common info\r");
        Serial.print( "--- Animal\r");
        Serial.print("name: Set/Get animal's name\r");
        Serial.print("sex: Set/Get gender: f or m\r");
        Serial.print( "--- LoraWAN\r");
        Serial.print("eui: Set/Get EUI of the LoraWAN module\r");
        Serial.print("nkey: Set/Get network key (only OTA supported)\r");
        Serial.print("akey: Set/Get app key (only OTA supported)\r");
        Serial.print( "--- Signals\r");
        Serial.print("sigl: execute signal Lights\r");
        Serial.print("sigs: execute signal Sound\r");
        Serial.print("sige: execute signal Electrick shock\r");
        Serial.print( "--- Hardware\r");
        Serial.print("led: enable/disable board LED\r");
        Serial.print("gps: status(e-emulated, y-hardware present, n-no hardware), current position, sattelites count\r");
        Serial.print("bat: Battery status\r");
        Serial.print("radio: Get LoRa RSSI, SNR and SS\r");
        Serial.print( "--- Program\r");
        Serial.print("flash: Flash values into peristent memory\r");
        Serial.print("restore: Restore values from peristent memory\r");
        Serial.print("reboot: Restart the device\r");
        Serial.print( "--- Legend\r");
        Serial.print( "EUI: LoRa device unique number\r");
        Serial.print( "RSSI: Received Signal Strength Indicator\r");
        Serial.print( "SNR: Signal-to-Noise Ratio\r");
        Serial.print( "SS: Signal Strength\r" );
        Serial.print("--------------------------\r\n");
        break;

    default:
        printHead();
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
        SerialCmd::sendCmd(SerialCmd::Cmd::Type::DBG, str);
        Serial.flush();
    }
}

void gPrintDbg(const char* chars) {
    gPrintDbg(String(chars));
}
