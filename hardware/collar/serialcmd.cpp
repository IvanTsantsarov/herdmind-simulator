#include "serialcmd.h"

#include "collar.h"
#include "led.h"
#include "defines.h"

#define SERIAL_TX_BUFFER_SIZE 1024
#define SERIAL_ARGUMENT_SIZE 32

#define SERIAL_CMD_DELIMETER(__char__) (__char__ == ' ')
#define SERIAL_CMD_END(__char__) (__char__ == '\r' || __char__ == '\n')


const char* SerialCmd::mCommands[] = { "" };

void SerialCmd::initCommands() {
    if( mCommands[0][0] ) {
        return;
    }

    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::NONE] = "none";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::DBG] = "dbg";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGL] = "sigl";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGS] = "sigs";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SIGE] = "sige";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::LED] = "led";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::HELP] = "help";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::REBOOT] = "reboot";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::INFO] = "info";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::EUI] = "eui";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::GPS] = "gps";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SAT] = "sat";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::BAT] = "bat";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::RSSI] = "rssi";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SNR] = "snr";
    SerialCmd::mCommands[(int)SerialCmd::Cmd::Type::SS] = "ss";
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


SerialCmd::SerialCmd(Collar* c) : mStrChanged("->")
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
    Serial.println("Setup collar...");
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
    String r = String(SERIAL_CMD_BEGIN) + cmd.typeStr() + (str.length() ? str : "") + SERIAL_CMD_TAIL;
    Serial.println(r);
}


#define SERIAL_UPDATE_ERROR(__err__)  Serial.println(String(SERIAL_CMD_BEGIN) + "error" + SERIAL_CMD_TAIL + __err__); clear(); skipAvailable(); return;

void SerialCmd::update()
{
    if( !mCommand ||!mCollar) {
        return;
    }

    while( Serial.available() ) {

        // Read and tap the buffer
        char ch = Serial.read();

        if( SERIAL_CMD_DELIMETER(ch)) {
            if( mIsArgument ) {
                SERIAL_UPDATE_ERROR("Second delimeter!");
            }

            mIsArgument = true;
            continue;
        }

        if( SERIAL_CMD_END(ch) ) {
            if( !mCmd.parse(mCommand, mCommandLen) ) {
                SERIAL_UPDATE_ERROR("Unknown command!");
            }

            if( mIsArgument && !hasArgumentString() ) {
                SERIAL_UPDATE_ERROR("Missing argument!");
            }

            if( !execute() ) {
                SERIAL_UPDATE_ERROR(mErrStr);
            }
            clear();
            return;
        }else {
            if( mIsArgument ) {

                if( mArgumentLen >= SERIAL_ARGUMENT_SIZE ) {
                    SERIAL_UPDATE_ERROR("Argument too long!");
                }

                mArgument[mArgumentLen++] = ch;
                mArgument[mArgumentLen] = 0;

            }else {
                if( mCommandLen >= SERIAL_CMD_SIZE ) {
                    SERIAL_UPDATE_ERROR("Command too long!");
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
    auto cmp = [&](const char* str){
        int len = 0;
        char ch = str[len];

        // make it lowercase
        if( ch >= 'A' && ch <= 'Z') {
            ch += ('a' - 'A');
        }

        while( ch && len < bufferLen)         {
            if( ch != buffer[len]){
                return false;
            }
            ch = str[++len];
        }
        return true;
    };

    if( cmp("help") ) {
         mT = Type::HELP;
    }else
    if( cmp("dbg") ) {
        mT = Type::DBG;
    }else
    if( cmp("led") ) {
        mT = Type::LED;
    }else
    if( cmp("reboot") ) {
        mT = Type::REBOOT;
    }else
    if( cmp("info") ) {
        mT = Type::INFO;
    }else
    if( cmp("ver") ) {
        mT = Type::VER;
    }else
    if( cmp("eui") ) {
        mT = Type::EUI;
    }else
    if( cmp("gps") ) {
        mT = Type::GPS;
    }else
    if( cmp("sat") ) {
        mT = Type::SAT;
    }else
    if( cmp("bat") ) {
        mT = Type::BAT;
    }else
    if( cmp("rssi") ) {
        mT = Type::RSSI;
    }else
    if( cmp("ss") ) {
        mT = Type::SS;
    }else
    if( cmp("snr") ) {
        mT = Type::SNR;
    }else
    if( cmp("nkey") ) {
        mT = Type::NKEY;
    }else
    if( cmp("akey") ) {
        mT = Type::AKEY;
    }else
    if( cmp("addr") ) {
        mT = Type::ADDR;
    }else
    if( cmp("fport") ) {
        mT = Type::FPORT;
    }else
    if( cmp("flash") ) {
        mT = Type::FLASH;
    }else
    if( cmp("restore") ) {
        mT = Type::RESTORE;
    }else
    if( cmp("name") ) {
        mT = Type::NAME;
    }else
    if( cmp("sex") ) {
        mT = Type::SEX;
    }
    else {
        mT = Type::NONE;
        return false;
    }

    return true;
}


bool SerialCmd::execute()
{
    auto printChanged = [&]() {
        Serial.println(mStrChanged + mArgument) ;
    };

    String head = String(SERIAL_CMD_BEGIN) + mCmd.typeStr() + SERIAL_CMD_TAIL;
    Serial.print(head);
    switch( mCmd.type() ) {
    case Cmd::Type::NONE:
        return false;
        break;
    case Cmd::Type::DBG:
        Serial.println(mCollar->toggleDebugInfo() ? "On" : "Off");
        break;
    case Cmd::Type::LED:
        Serial.println(mCollar->led()->toggle() ? "On" : "Off");
        break;
    case Cmd::Type::REBOOT:
        mCollar->reboot();
        break;
    case Cmd::Type::INFO:
        Serial.println( mCollar->animalName() + SERIAL_CMD_PARAMS_DM +
                       (mCollar->isMale() ? "m":"f") + SERIAL_CMD_PARAMS_DM +
                        mCollar->euiHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->akeyHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->nkeyHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->addrHex() + SERIAL_CMD_PARAMS_DM +
                        mCollar->fport() );
        break;

    case Cmd::Type::GPS: {
        Serial.println( mCollar->gpsStr() );
    }
    break;

    case Cmd::Type::SAT:
        Serial.println( mCollar->satellites() );
        break;

    case Cmd::Type::BAT:
        Serial.println( mCollar->batteryInfo() );
        break;

    case Cmd::Type::RSSI:
        Serial.println( mCollar->rssi() );
        break;
    case Cmd::Type::SNR:
        Serial.println( mCollar->snr() );
        break;
    case Cmd::Type::SS:
        Serial.println( mCollar->signalStrength() );
        break;

    case Cmd::Type::NKEY:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                mErrStr = "Wrong netkey length!";
                return false;
            }
            mCollar->setNKeyHex(mArgument);
            printChanged();
        }else {
            Serial.println(mCollar->nkeyHex());
        }
        break;

    case Cmd::Type::AKEY:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                mErrStr = "Wrong AppKey lenght!";
                return false;
            }
            mCollar->setAKeyHex(mArgument);
            printChanged();
        }else {
            Serial.println(mCollar->akeyHex());
        }
        break;

    case Cmd::Type::NAME:
        if( mIsArgument ) {
            if( mArgumentLen < 3) {
                mErrStr = "Wrong name lenght!";
                return false;
            }
            mCollar->setAnimalName(mArgument);
            printChanged();
        }else {
            Serial.println(mCollar->animalName());
        }
        break;

    case Cmd::Type::SEX:
        if( mIsArgument ) {
            if( mArgumentLen != 1) {
                mErrStr = "Wrong sex lenght!";
                return false;
            }
            mCollar->setIsMale(strcmp(mArgument,"m") == 0 ? true : false);
            Serial.println(mStrChanged + (mCollar->isMale() ? "male" : "female") );
        }else {
            Serial.println(mCollar->isMale() ? "male" : "female");
        }
        break;

    case Cmd::Type::EUI:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_EUI_HEX_LEN) {
                mErrStr = "Wrong EUI lenght!";
                return false;
            }
            mCollar->setEuiHex(mArgument);
            printChanged();
        }else {
            Serial.println(mCollar->euiHex());
        }
        break;

    case Cmd::Type::ADDR:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_ADDR_HEX_LEN) {
                mErrStr = "Wrong Address lenght!";
                return false;
            }
            mCollar->setAddrHex(mArgument);
            printChanged();
        }else {
            Serial.println(mCollar->addrHex());
        }
        break;

    case Cmd::Type::FPORT:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_FPORT_HEX_LEN) {
                mErrStr = "Wrong FPort lenght!";
                return false;
            }
            mCollar->setFPortHex(mArgument);
            printChanged();
        }else {
            Serial.println(String("0x") + mCollar->fportHex());
        }
        break;

    case Cmd::Type::HELP:
        Serial.print("reset: Restarts the ESP32\r");
        Serial.print("dbg: Toggle debug info\r");
        Serial.print("led: enable/disable board LED\r");
        Serial.print("help: This help\r");
        Serial.print("info: Common info\r");
        Serial.print("gps: Current geo position\r");
        Serial.print("sat: Count of available GPS sattelites\r");
        Serial.print("bat: Battery status\r");
        Serial.print("rssi: Received Signal Strenght Indicator in dB\r");
        Serial.print("snr: Signal to Noise Ratio in dB\r");
        Serial.print("ss: Signal Strength in percents\r");
        Serial.print("name: Set/Get animal's name\r");
        Serial.print("sex: Set/Get gender: f or m\r");
        Serial.print("eui: Set/Get EUI of the LoraWAN module\r");
        Serial.print("nkey: Set/Get network key (only OTA supported)\r");
        Serial.print("akey: Set/Get app key (only OTA supported)\r");
        Serial.print("flash: Flash values into peristent memory\r");
        Serial.print("restore: Restore values from peristent memory\r");
        Serial.print("reboot: Restart the device\r");
        Serial.println("");
        break;

    case Cmd::Type::FLASH:
        if( !mCollar->flash() ) {
            Serial.println("Error!");
        }else {
            Serial.println("Done.");
        }
        break;
    case Cmd::Type::RESTORE:
        if( !mCollar->restore() ) {
            Serial.println("Error!");
        }else {
            Serial.println("Done.");
        }
        break;

    case Cmd::Type::VER:
        Serial.println(COLLAR_VERSION);
        break;
    case Cmd::Type::RSP:
        // not receiving - just sending. Here because of the warning
        break;
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

