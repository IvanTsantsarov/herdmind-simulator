#include "serialcmd.h"

#include "collar.h"

#ifdef SIMULATION
    #include "../tools.h"
#else
    #include <Arduino.h>
#endif


#define SERIAL_BUFFER_SIZE 127

#define SERIAL_CMD_DELIMETER(__char__)__char__ == ' '
#define SERIAL_CMD_END(__char__) __char__ == '\r' __char__ == '\n'

SerialCmd::SerialCmd(Collar* c)
{
    mCollar = c;

    // Flush the old content
    while(Serial.available());

    // Create the buffer
    mBuffer = new char[SERIAL_BUFFER_SIZE + 1];
    mBuffer[0] = 0;
}

SerialCmd::~SerialCmd()
{
    if( mBuffer) {
        delete [] mBuffer;
        mBuffer = 0;
        mBufferLen = 0;
    }
}

void SerialCmd::clear()
{
    mCmd.clear();
    mBuffer[0] = 0;
    mBufferLen = 0;
}

#define SERIAL_UPDATE_ERROR(__err__) Serial.println(String("Error:") + __err__); clear(); return;

void SerialCmd::update()
{
    if( !mBuffer ||!mCollar) {
        return;
    }

    while(Serial.available() && mBufferLen < SERIAL_BUFFER_SIZE) {

        // Read and tap the buffer
        char ch = Serial.read();
        mBuffer[mBufferLen++] = ch;
        mBuffer[mBufferLen] = 0;

        if( SERIAL_CMD_DELIMETER(ch) ) {
            if( mCmd.isNone() ) {
                SERIAL_UPDATE_ERROR("Command is missing!");
            }

            if( !mCmd.parse(mBuffer, mBufferLen) ) {
                SERIAL_UPDATE_ERROR("Unknown command!");
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
            ch = str[len++];
        }
        return true;
    };

    if( cmp("restart") ) {
        mT = Type::RESTART;
    }else
    if( cmp("info") ) {
        mT = Type::INFO;
    }else
    if( cmp("eui") ) {
        mT = Type::EUI;
    }
    if( cmp("gps") ) {
        mT = Type::EUI;
    }else
    if( cmp("bat") ) {
        mT = Type::EUI;
    }
    else {
        mT = Type::NONE;
        return false;
    }

    return true;
}


void SerialCmd::execute()
{
    switch( mCmd.type() ) {
    case Cmd::Type::RESTART:
        mCollar->restart();
        break;
    case Cmd::Type::BAT:
        Serial.println(mCollar->batteryLevel());
        break;
    case Cmd::Type::EUI:
        Serial.println(mCollar->eui().toBase64().data());
        break;

    case Cmd::Type::NONE:
    case Cmd::Type::INFO:
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

    case Cmd::Type::RSSI:
        Serial.println( mCollar->rssi() );
        break;
    case Cmd::Type::SNR:
        Serial.println( mCollar->snr() );
        break;
    case Cmd::Type::SS:
        Serial.println( mCollar->signalStrength() );
        break;
    }
}

