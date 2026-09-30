#include "serialcmd.h"

#include "collar.h"
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
    if( cmp("restart") ) {
        mT = Type::RESTART;
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
    }

    else {
        mT = Type::NONE;
        return false;
    }

    return true;
}


bool SerialCmd::execute()
{
    switch( mCmd.type() ) {
    case Cmd::Type::NONE:
        Serial.println( "Error:Empty command!" );
        break;
    case Cmd::Type::RESTART:
        mCollar->restart();
        break;
    case Cmd::Type::INFO:
        Serial.println("Info will be added later.");
        break;

    case Cmd::Type::GPS: {
        GeoPoint pos = mCollar->gps();
        String lat (pos.mLat, 10);
        String lon (pos.mLon, 10);
        Serial.println( lat + "," + lon );
    }
    break;

    case Cmd::Type::SAT:
        Serial.println( mCollar->satellites() );
        break;

    case Cmd::Type::BAT:
        Serial.println( mCollar->batteryLevel() );
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
            if( mArgumentLen != LORA_KEY_LEN) {
                mErrStr = "Wrong netkey length";
                return false;
            }
            mCollar->setNKey(mArgument);
            Serial.println("nkey ok");
        }else {
            Serial.println(mCollar->nkeyStr());
        }
        break;

    case Cmd::Type::AKEY:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_KEY_LEN) {
                mErrStr = "Wrong AppKey lenght";
                return false;
            }
            mCollar->setAKey(mArgument);
            Serial.println("akey ok");
        }else {
            Serial.println(mCollar->akeyStr());
        }
        break;

    case Cmd::Type::EUI:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_EUI_HEX_LEN) {
                mErrStr = "Wrong EUI lenght";
                return false;
            }
            mCollar->setEui(mArgument);
            Serial.println("eui ok");
        }else {
            Serial.println(mCollar->euiStr());
        }
        break;

    case Cmd::Type::HELP:
        Serial.println("restart: Restarts the ESP32");
        Serial.println("help: This help");
        Serial.println("info: Common info");
        Serial.println("gps: Current geo position");
        Serial.println("sat: Count of available GPS sattelites");
        Serial.println("bat: Battery level");
        Serial.println("rssi: Received Signal Strenght Indicator in dB");
        Serial.println("snr: Signal to Noise Ratio in dB");
        Serial.println("ss: Signal Strength in percents");
        Serial.println("eui: Set/Get EUI of the LoraWAN module");
        Serial.println("nkey: Set/Get network key (only OTA supported)");
        Serial.println("akey: Set/Get app key (only OTA supported)");
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

