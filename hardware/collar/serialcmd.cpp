#include "serialcmd.h"

#include "collar.h"
#include "led.h"
#include "defines.h"

#define SERIAL_COMMAND_SIZE 7

#define SERIAL_ARGUMENT_SIZE 32

#define SERIAL_CMD_DELIMETER(__char__) (__char__ == ' ')
#define SERIAL_CMD_END(__char__) (__char__ == '\r' || __char__ == '\n')

SerialCmd::SerialCmd(Collar* c)
{
    mCollar = c;

    // Create the buffer
    mCommand = new char[SERIAL_COMMAND_SIZE + 1];
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

void SerialCmd::clear()
{
    mCmd.clear();
    mCommand[0] = 0;
    mCommandLen = 0;
    mArgument[0] = 0;
    mArgumentLen = 0;
    mIsArgument = false;

}

#define SERIAL_UPDATE_ERROR(__err__)  Serial.println(String("Error:") + __err__); clear(); skipAvailable(); return;

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
                SERIAL_UPDATE_ERROR("Second delimeter");
            }

            mIsArgument = true;
            continue;
        }

        if( SERIAL_CMD_END(ch) ) {
            if( !mCmd.parse(mCommand, mCommandLen) ) {
                SERIAL_UPDATE_ERROR("Unknown command");
            }

            if( mIsArgument && !hasArgumentString() ) {
                SERIAL_UPDATE_ERROR("Missing argument");
            }

            if( !execute() ) {
                SERIAL_UPDATE_ERROR(mErrStr);
            }
            clear();
            return;
        }else {
            if( mIsArgument ) {

                if( mArgumentLen >= SERIAL_ARGUMENT_SIZE ) {
                    SERIAL_UPDATE_ERROR("Argument too long");
                }

                mArgument[mArgumentLen++] = ch;
                mArgument[mArgumentLen] = 0;

            }else {
                if( mCommandLen >= SERIAL_COMMAND_SIZE ) {
                    SERIAL_UPDATE_ERROR("Command too long");
                }
                mCommand[mCommandLen++] = ch;
                mCommand[mCommandLen] = 0;
            }
        }
    }
}


const char *SerialCmd::Cmd::typeStr()
{
    switch(mT) {
    case Type::NONE: return "none";
    case Type::DBG: return "dbg";
    case Type::LED: return "led";
    case Type::HELP: return "help";
    case Type::RESET: return "reset";
    case Type::INFO: return "info";
    case Type::EUI: return "eui";
    case Type::GPS: return "gps";
    case Type::SAT: return "sat";
    case Type::BAT: return "bat";
    case Type::RSSI: return "rssi";
    case Type::SNR: return "snr";
    case Type::SS: return "ss";
    case Type::NKEY: return "nkey";
    case Type::AKEY: return "akey";
    case Type::ADDR: return "addr";
    case Type::FLASH: return "flash";
    case Type::RESTORE: return "restore";
    }

    return "";
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
    if( cmp("restart") ) {
        mT = Type::RESET;
    }else
    if( cmp("info") ) {
        mT = Type::INFO;
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
    if( cmp("flash") ) {
        mT = Type::FLASH;
    }else
    if( cmp("restore") ) {
        mT = Type::RESTORE;
    }

    else {
        mT = Type::NONE;
        return false;
    }

    return true;
}


bool SerialCmd::execute()
{
    String head = String(SERIAL_CMD_BEGIN) + mCmd.typeStr() + SERIAL_CMD_TAIL;
    Serial.print(head);
    switch( mCmd.type() ) {
    case Cmd::Type::NONE:
        Serial.println( "Error:Empty command!" );
        break;
    case Cmd::Type::DBG:
        Serial.println(String("Debug info ") + (mCollar->toggleDebugInfo() ? "ON" : "OFF"));
        break;
    case Cmd::Type::LED:
        Serial.println(String("LED is") + (mCollar->led()->toggle() ? "ON" : "OFF"));
        break;
    case Cmd::Type::RESET:
        mCollar->restart();
        break;
    case Cmd::Type::INFO:
        Serial.println( mCollar->animalName() + SERIAL_CMD_PARAMS_DM +
                       (mCollar->isMale() ? "m":"f") + SERIAL_CMD_PARAMS_DM +
                       mCollar->euiHex() + SERIAL_CMD_PARAMS_DM +
                       mCollar->akeyHex() + SERIAL_CMD_PARAMS_DM +
                       mCollar->nkeyHex() );
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
                mErrStr = "Wrong netkey length";
                return false;
            }
            mCollar->setNKeyHex(mArgument);
            Serial.println("nkey ok");
        }else {
            Serial.println(mCollar->nkeyHex());
        }
        break;

    case Cmd::Type::AKEY:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                mErrStr = "Wrong AppKey lenght";
                return false;
            }
            mCollar->setAKeyHex(mArgument);
            Serial.println("akey ok");
        }else {
            Serial.println(mCollar->akeyHex());
        }
        break;

    case Cmd::Type::EUI:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_EUI_HEX_LEN) {
                mErrStr = "Wrong EUI lenght";
                return false;
            }
            mCollar->setEuiHex(mArgument);
            Serial.println("eui ok");
        }else {
            Serial.println(mCollar->euiHex());
        }
        break;

    case Cmd::Type::HELP:
        Serial.println("reset: Restarts the ESP32");
        Serial.println("dbg: enable/disable debug info");
        Serial.println("led: enable/disable board LED");
        Serial.println("help: This help");
        Serial.println("info: Common info");
        Serial.println("gps: Current geo position");
        Serial.println("sat: Count of available GPS sattelites");
        Serial.println("bat: Battery status");
        Serial.println("rssi: Received Signal Strenght Indicator in dB");
        Serial.println("snr: Signal to Noise Ratio in dB");
        Serial.println("ss: Signal Strength in percents");
        Serial.println("eui: Set/Get EUI of the LoraWAN module");
        Serial.println("nkey: Set/Get network key (only OTA supported)");
        Serial.println("akey: Set/Get app key (only OTA supported)");
        Serial.println("flash: Flash values into peristent memory");
        Serial.println("restore: Restore values from peristent memory");
        break;

    case Cmd::Type::ADDR:
        break;
    case Cmd::Type::FLASH:
        if( !mCollar->flash() ) {
            Serial.println("Error flashing.");
        }else {
            Serial.println("Flashing done.");
        }
        break;
    case Cmd::Type::RESTORE:
        if( !mCollar->restore() ) {
            Serial.println("Error restoring.");
        }else {
            Serial.println("Restoring done.");
        }
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

