#include "cli.h"

#include "collar.h"
#include "led.h"
#include "ble.h"
#include "defines.h"

#define SERIAL_TX_BUFFER_SIZE 1024
#define SERIAL_ARGUMENT_SIZE 32

#define CLI_CMD_IS_SPACE(__char__) (__char__ == ' ')
#define CLI_CMD_END(__char__) (__char__ == '\r' || __char__ == '\n')

#define CLI_CMD_STRING_CHANGED "->"

const char* CLI::mCommands[] = { "" };

void CLI::initCommands() {
    if( mCommands[0][0] ) {
        return;
    }

    CLI::mCommands[(int)CLI::Cmd::Type::ADDR] = "addr";
    CLI::mCommands[(int)CLI::Cmd::Type::AKEY] = "akey";
    CLI::mCommands[(int)CLI::Cmd::Type::BAT] = "bat";
    CLI::mCommands[(int)CLI::Cmd::Type::DBG] = "dbg";
    CLI::mCommands[(int)CLI::Cmd::Type::EUI] = "eui";
    CLI::mCommands[(int)CLI::Cmd::Type::FENCE] = "fence";
    CLI::mCommands[(int)CLI::Cmd::Type::FLASH] = "flash";
    CLI::mCommands[(int)CLI::Cmd::Type::FPORT] = "fport";
    CLI::mCommands[(int)CLI::Cmd::Type::GPS] = "gps";
    CLI::mCommands[(int)CLI::Cmd::Type::HELP] = "help";
    CLI::mCommands[(int)CLI::Cmd::Type::INFO] = "info";
    CLI::mCommands[(int)CLI::Cmd::Type::INT] = "int";
    CLI::mCommands[(int)CLI::Cmd::Type::LED] = "led";
    CLI::mCommands[(int)CLI::Cmd::Type::MEM] = "mem";
    CLI::mCommands[(int)CLI::Cmd::Type::NAME] = "name";
    CLI::mCommands[(int)CLI::Cmd::Type::NKEY] = "nkey";
    CLI::mCommands[(int)CLI::Cmd::Type::NONE] = "none";
    CLI::mCommands[(int)CLI::Cmd::Type::RADIO] = "radio";
    CLI::mCommands[(int)CLI::Cmd::Type::REBOOT] = "reboot";
    CLI::mCommands[(int)CLI::Cmd::Type::RESTORE] = "restore";
    CLI::mCommands[(int)CLI::Cmd::Type::RSP] = "rsp";
    CLI::mCommands[(int)CLI::Cmd::Type::RTM] = "rtm";
    CLI::mCommands[(int)CLI::Cmd::Type::SEX] = "sex";
    CLI::mCommands[(int)CLI::Cmd::Type::SIGE] = "sige";
    CLI::mCommands[(int)CLI::Cmd::Type::SIGL] = "sigl";
    CLI::mCommands[(int)CLI::Cmd::Type::SIGS] = "sigs";
    CLI::mCommands[(int)CLI::Cmd::Type::STATS] = "stats";
    CLI::mCommands[(int)CLI::Cmd::Type::SLEEP] = "sleep";
    CLI::mCommands[(int)CLI::Cmd::Type::TM] = "tm";
    CLI::mCommands[(int)CLI::Cmd::Type::VER] = "ver";
}


CLI::CLI(Collar* c)
{
    CLI::initCommands();

    mCollar = c;

    // Create the buffer
    mCommand = new char[CLI_CMD_SIZE + 1];
    mArgument = new char[SERIAL_ARGUMENT_SIZE + 1];
    clear();
}

CLI::~CLI()
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

void CLI::setup(Ble *ble)
{
    if( !ble) {
        // allocate TX buffer for non-blocking sends
        Serial.setTxBufferSize(SERIAL_TX_BUFFER_SIZE);

        Serial.begin(SERIAL_BAUDRATE);

        /////////////////////////////////////////////////
        // Do not block here, because if it's disconected
        // from USB it will never start !
        // while(!Serial);

        delay(100);
        DBG("[CLI]: Serial setup done.");
    }else {
        DBG("[CLI]: Bluetooth setup done.");
    }

    DBG("[CLI] Print \"help\" for more info...");
}

void CLI::clear()
{
    mCmd.clear();
    mCommand[0] = 0;
    mCommandLen = 0;
    mArgument[0] = 0;
    mArgumentLen = 0;
    mIsArgument = false;

}

void CLI::sendCommand(Cmd::Type t, const String& str)
{
    Cmd cmd(t);
    String r = String(CLI_CMD_BEGIN) + Cmd::type2StrStatic(t) + CLI_CMD_TAIL + str;

    if( hasBle() ) {
        mBle->send(r);
    }else {
        Serial.println(r);
        Serial.flush();
    }
}


void CLI::sendCommandSerial(Cmd::Type t, const String& str)
{
    Cmd cmd(t);
    String r = String(CLI_CMD_BEGIN) + Cmd::type2StrStatic(t) + CLI_CMD_TAIL + str;
    Serial.println(r);
    Serial.flush();
}


#define SERIAL_UPDATE_ERROR(__err__)  sendError(String(mCommand) + ":" + (__err__)); clear(); skipAvailable(); return;

void CLI::update()
{

    if( !mCommand ||!mCollar) {
        return;
    }

    auto dataAvailable = [&]() {
        return hasBle() ?  mBle->available() : Serial.available();
    };

    auto dataRead = [&]() {
        return hasBle() ?  mBle->read() : Serial.read();
    };

    while( dataAvailable() ) {

        // Read and tap the buffer
        char ch = dataRead();

        if( CLI_CMD_IS_SPACE(ch)) {
            if( mIsArgument ) {
                SERIAL_UPDATE_ERROR("Second delimeter!");
            }

            mIsArgument = true;
            continue;
        }

        if( CLI_CMD_END(ch) ) {
            if( !mCmd.parse(mCommand, mCommandLen) ) {
                SERIAL_UPDATE_ERROR("Unknown command!");
            }

            if( mIsArgument && !hasArgumentString() ) {
                SERIAL_UPDATE_ERROR("Missing argument!");
            }

            execute();
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
                if( mCommandLen >= CLI_CMD_SIZE ) {
                    SERIAL_UPDATE_ERROR("Command too long!");
                }
                mCommand[mCommandLen++] = ch;
                mCommand[mCommandLen] = 0;
            }
        }
    }
}


const char *CLI::Cmd::typeStr()
{
    return CLI::Cmd::type2StrStatic(this->mT);
}

const char* CLI::Cmd::type2StrStatic(Type t)
{
    assert(t < CLI::Cmd::Type::LAST);
    return mCommands[(int)t];
}

CLI::Cmd::Type CLI::Cmd::str2TypeStatic(const char* str)
{
    for( auto t = Type::NONE; t < Type::LAST; t = Type((int)t + 1)) {
        if( !strcmp(type2StrStatic(t), str) ) {
            return t;
        }
    }

    return Type::NONE;
}

bool CLI::Cmd::parse(const char *buffer, int bufferLen)
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



bool CLI::execute()
{
    auto printChanged = [&]() {
        sendResponce(String(CLI_CMD_STRING_CHANGED) + mArgument);
    };

    auto responceCommand = [&](const String& str) {
        sendCommand(mCmd.type(), str);
    };

    switch( mCmd.type() ) {
    case Cmd::Type::NONE:
        return false;
        break;
    case Cmd::Type::DBG:
        sendResponce(mCollar->toggleDbg() ? "Debug On" : "Debug Off");
        break;
    case Cmd::Type::STATS:
        sendResponce(mCollar->toggleStats() ? "Stats On" : "Stats Off");
        break;
    case Cmd::Type::LED:
        sendResponce(mCollar->led()->toggle() ? "LED On" : "LED Off");
        break;
    case Cmd::Type::REBOOT:
        sendResponce("Rebooting...");
        mCollar->reboot();
        break;
    case Cmd::Type::INFO:
    {
        responceCommand( String(mCollar->animalName()) + CLI_CMD_PARAMS_DM +
                       (mCollar->isMale() ? "m":"f") + CLI_CMD_PARAMS_DM +
                        mCollar->euiHex() + CLI_CMD_PARAMS_DM +
                        mCollar->akeyHex() + CLI_CMD_PARAMS_DM +
                        mCollar->nkeyHex() + CLI_CMD_PARAMS_DM +
                        mCollar->addrHex() + CLI_CMD_PARAMS_DM +
                        mCollar->fport() );
        break;
    }

    case Cmd::Type::GPS: {
        sendResponce( String("gps=") + mCollar->gpsStatus() + CLI_CMD_PARAMS_DM +
                        "lat=" + String(mCollar->gpsPos().mLat, 10) + CLI_CMD_PARAMS_DM +
                        "lon=" + String(mCollar->gpsPos().mLon, 10) + CLI_CMD_PARAMS_DM +
                        "sat=" + mCollar->satellites() );
    }
    break;


    case Cmd::Type::BAT:
        responceCommand( mCollar->batteryInfo() );
        break;

    case Cmd::Type::RADIO:
        responceCommand( String("RSSI=") + mCollar->rssi() + CLI_CMD_PARAMS_DM
                       + "SNR=" + mCollar->snr() + CLI_CMD_PARAMS_DM
                       + "SS=" + mCollar->signalStrength());
        break;


    case Cmd::Type::INT:
    {
        if( !mIsArgument ) {
            responceCommand( String("Send/sleep inverval is: ") + mCollar->sendInterval() + " msec" );
            return true;
        }


        const uint64_t msec = strtoull(mArgument, NULL, 10);
        if( msec < 1000) {
            sendError("Interval is less than a second.");
            return false;
        }
        mCollar->setSendInterval(msec);
        sendResponce( String("Send/sleep interval set to:") + msec + "msec" );
        break;
    }

    case Cmd::Type::SLEEP:
    {
        if( mIsArgument ) {
            char argChar = mArgument[0];
            bool is = (argChar == 'y' || argChar == '1' ? true : false);
            mCollar->enableDeepSleep(is);
        }

        sendResponce( String("Deep sleep is:") + (mCollar->isDeepSleep() ? "Enabled" : "Disabled") );
        break;
    }
    case Cmd::Type::RTM:
        // Time sync requect
        sendCommandSerial(Cmd::Type::RTM);
        mIsWaitingForTimeSync = true;
        break;
    case Cmd::Type::TM:
        mIsWaitingForTimeSync = false;

        if( mIsArgument ) {
            const uint64_t secsEpoch = strtoull(mArgument, NULL, 10);
            if( secsEpoch < 1767225600) { // epoch of 01.01.20026
                sendError("Wrong time (before 01.01.2026)!");
                return false;
            }

            int64_t diff = (int64_t)mCollar->epoch() - (int64_t)secsEpoch;

            if( diff ) {
                mCollar->setEpoch(secsEpoch);
                sendResponce( String("Time set to:") + mCollar->dateTime() +
                    (diff < 0 ? "\rSecs slower:" : "\rSecs faster:") + abs(diff) );
            }else {
                sendResponce( "Time is the same." );
            }
        }else {
            responceCommand(mCollar->dateTime());
        }
        break;

    case Cmd::Type::NKEY:

        if( mIsArgument ) {
            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                sendError( "Wrong netkey length!" );
                return false;
            }
            mCollar->setNKeyHex(mArgument);
            printChanged();
        }else {
            responceCommand(mCollar->nkeyHex());
        }
        break;

    case Cmd::Type::AKEY:
        if( mIsArgument ) {

            if( mArgumentLen != LORA_KEY_HEX_LEN) {
                sendError("Wrong AppKey lenght!");
                return false;
            }
            mCollar->setAKeyHex(mArgument);
            printChanged();
        }else {
            responceCommand(mCollar->akeyHex());
        }
        break;

    case Cmd::Type::NAME:
        if( mIsArgument ) {
            if( mArgumentLen < 3) {
                sendError("Wrong name lenght!");
                return false;
            }
            mCollar->setAnimalName(mArgument);
            printChanged();
        }else {
            responceCommand(mCollar->animalName());
        }
        break;

    case Cmd::Type::SEX:
        if( mIsArgument ) {
            if( mArgumentLen != 1) {
                sendError("Wrong sex lenght!");
                return false;
            }
            mCollar->setIsMale(strcmp(mArgument,"m") == 0 ? true : false);
            printChanged();
        }else {
            responceCommand(mCollar->isMale() ? "m" : "f");
        }
        break;

    case Cmd::Type::EUI:
        if( mIsArgument ) {

            if( mArgumentLen != LORA_EUI_HEX_LEN) {
                sendError("Wrong EUI lenght!");
                return false;
            }
            mCollar->setEuiHex(mArgument);
            printChanged();
        }else {
            responceCommand(mCollar->euiHex());
        }
        break;

    case Cmd::Type::ADDR:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_ADDR_HEX_LEN) {
                sendError("Wrong Address lenght!");
                return false;
            }
            mCollar->setAddrHex(mArgument);
            printChanged();
        }else {
            responceCommand(mCollar->addrHex());
        }
        break;

    case Cmd::Type::FPORT:
        if( mIsArgument ) {
            if( mArgumentLen != LORA_FPORT_HEX_LEN) {
                sendError("Wrong FPort lenght!");
                return false;
            }
            mCollar->setFPortHex(mArgument);
            printChanged();
        }else {
            responceCommand(String("0x") + mCollar->fportHex());
        }
        break;

    case Cmd::Type::FLASH:
        if( !mCollar->flash() ) {
            sendError("Flashing.");
        }else {
            sendResponce("Flashing done.");
        }
        break;
    case Cmd::Type::RESTORE:

        if( !mCollar->restore() ) {
            sendError("Restoring.");
        }else {
            sendResponce("Restoring done.");
        }
        break;

    case Cmd::Type::VER:
        responceCommand(COLLAR_VERSION);
        break;

    case Cmd::Type::RSP:
        // not receiving - just sending. Here because of the warning
        break;
    case Cmd::Type::SIGL:
        sendResponce("Executing lights...");
        mCollar->execSignal(Collar::SignalType::Light);
        break;
    case Cmd::Type::SIGS:
        sendResponce("Executing sound...");
        mCollar->execSignal(Collar::SignalType::Sound);
        break;
    case Cmd::Type::SIGE:
        sendResponce("Executing electric shock...");
        mCollar->execSignal(Collar::SignalType::Shock);
        break;
    case Cmd::Type::MEM:
        responceCommand( mCollar->getMemInfo() );
        break;
    case Cmd::Type::HELP:
        responceCommand(
        "---------- HELP ----------\r"
        "stats: Toggle statistics\r"
        "info: Common info\r"
        "mem: Get available memory\r"
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
        "sigl: Execute signal Lights\r"
        "sigs: Execute signal Sound\r"
        "sige: Execute signal Electrick shock\r"
        "fence: Enable/Disable fence and Get/Set fence points. Usage: fence 0/1|point_lat,point_lon|...\r"
         "--- Hardware\r"
        "led: Enable/disable board LED\r"
        "gps: Status(e-emulated, y-hardware present, n-no hardware), current position, sattelites count\r"
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
        sendError( String("Wrong command:") + mCmd.typeStr() );
        return false;
    }

    return true;
}

void CLI::skipAvailable()
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
        CLI::sendCommandSerial(CLI::Cmd::Type::DBG, str);
        Serial.flush();
    }
}

void gPrintDbg(const char* chars) {
    gPrintDbg(String(chars));
}
