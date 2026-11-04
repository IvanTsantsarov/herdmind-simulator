#include "loradev_hw.h"

#include <cstdio>

/*
// EU868
Data Rate (DR)	Configuration	Bit Rate	Typical Use / Characteristics
    DR0         SF12 / 125 kHz	250 bit/s	Longest range, slowest speed
    DR1         SF11 / 125 kHz	440 bit/s	High range
    DR2         SF10 / 125 kHz	980 bit/s	Medium-high range
    DR3         SF9 / 125 kHz	1,760 bit/s	Medium range
    DR4         SF8 / 125 kHz	3,125 bit/s	Shorter range
    DR5         SF7 / 125 kHz	5,470 bit/s	Short range, fast
    DR6         SF7 / 250 kHz	11,000 bit/s	High speed, wide band
    DR7         FSK / 50 kbps	50,000 bit/s	Frequency Shift Keying (rarely used)
*/

#define PIN_NSS   8
#define PIN_SCK   9
#define PIN_MOSI  10
#define PIN_MISO  11
#define PIN_DIO1  14
#define PIN_NRST  12
#define PIN_BUSY  13

extern String gRadioLibErrStr(int16_t errorCode);

// Msec to wait additionaly for the RX window
#define LORAWAN_RX_WINDOW 250

LoraDevHW::LoraDevHW() {
}

void LoraDevHW::onSetup() {
    uint8_t mac[6] = {0};

    // Get the base MAC address of the ESP32 (6 bytes unique)
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    // Convert 6-byte MAC into an 8-byte LoRaWAN DevEUI
    // Standard approach expands the middle with 0xFF, 0xFE or padding
    mEui[0] = mac[0];
    mEui[1] = mac[1];
    mEui[2] = mac[2];
    mEui[3] = 0xFF; // Padding byte
    mEui[4] = 0xFE; // Padding byte
    mEui[5] = mac[3];
    mEui[6] = mac[4];
    mEui[7] = mac[5];

    DBG(String("[LoraDevHW] === Setup start ==="));
    DBG(String("[LoraDevHW] Build:") + __TIME__ + "," + __DATE__);
    DBG(String("[LoraDevHW] Addr:") + addrHex() + ", fport:" + fportHex());
    DBG(String("[LoraDevHW] NKey:") + nkeyHex());
    DBG(String("[LoraDevHW] AKey:") + akeyHex());


    // 1. Map the exact Heltec WiFi LoRa 32 V4.3 internal hardware pins
    // Pins for V4.3: NSS=8, DIO1=14, NRST=12, BUSY=13
    mRadio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);

    // 2. Create the LoRaWAN Node Instance
    mNode = new LoRaWANNode(&mRadio, &EU868); // Automatically sets up EU868 frequencies

    // disable frame counter check to stop MIC error
    // mNode->frameCounterCheck = false;


    //////////////////////////////////////////////////////
    ///  IMPORTANT!!!! Without this downlink is impossible
    if( LORAWAN_RX_WINDOW ) {
        mNode->scanGuard = LORAWAN_RX_WINDOW;
    }
    DBG(String("[LoraDevHW] scanGuard=") + mNode->scanGuard);
    ////////////////////////////////////////

    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_NSS);

    ConfigLoRa_t config{};
    config.frequency = 868; // The frequency here does not matter, as it will get changed by LoRaWAN anyway

    int state = mRadio.begin(config);
    if (state != RADIOLIB_ERR_NONE) {
        DBG( String("[LoraDevHW] ERROR:Lora init failed!") + gRadioLibErrStr(state));
        return;
    }

    // Pull in the ABP profiles natively
    state = mNode->beginABP(mAddr, nullptr, nullptr, mNKey, mAKey);

    if (state != RADIOLIB_ERR_NONE) {
        DBG( String("[LoraDevHW] ERROR:beginABP failed: ") + gRadioLibErrStr(state));
        return;
    }

    // Configure RX2 before activating the session.
    //state = mNode->setRx2Dr(0);
    //DBG(String("[LoraDevHW] setRx2Dr(0):") + state);

    state = mNode->activateABP();

    if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
        DBG("[LoraDevHW] ABP activated: new session");
    } else if (state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
        DBG("[LoraDevHW] ABP activated: session restored");
    } else if (state != RADIOLIB_ERR_NONE) {
        DBG(String("[LoraDevHW] ERROR: activateABP failed:") + gRadioLibErrStr(state) );
        return;
    }

    DBG(String("[LoraDevHW] === Setup end ==="));
}

void LoraDevHW::sendPackage(void *data, size_t len)
{

    if( len > LORAWAN_APP_DATA_MAX_SIZE ) {
        DBG( String("[LoraDevHW] ERROR: send data > LORAWAN_APP_DATA_MAX_SIZE:") + len);
        len = LORAWAN_APP_DATA_MAX_SIZE;
    }
    const char* charPtr = ((char*)data);
    // memcpy(appData, data, static_cast<size_t>(len));
    // appDataSize = static_cast<uint8_t>(len);
    for( mBufferUpLen = 0; mBufferUpLen < len; mBufferUpLen ++) {
        mBufferUp[mBufferUpLen] = charPtr[mBufferUpLen];
    }

    DBG(String("[LoraDevHW] Sending:") + toHex(mBufferUp, mBufferUpLen) + " on port:" + mFPort);

    // Fires an unconfirmed uplink packet directly on specified fport
    // int state = mNode->sendReceive( mBufferUp, mBufferUpLen, mFPort, false);
    mBufferDownLen = LORAWAN_APP_DATA_MAX_SIZE;
    int state = mNode->sendReceive( mBufferUp, mBufferUpLen, mFPort,
                                   mBufferDown, &mBufferDownLen,
                                   false);

    DBG(String("[LoraDevHW] Downlink size:") + mBufferDownLen);

    if (state >= RADIOLIB_ERR_NONE) {
        DBG( String("[LoraDevHW] Uplink done with state:") + gRadioLibErrStr(state));

        if (state > 0) {
            DBG("[LoraDevHW] Downlink received");
            DBG( toHex(mBufferDown, mBufferDownLen) );
            onReceive(mBufferDown, mBufferDownLen);
        } else {
            DBG("[LoraDevHW] No downlink received");
        }
    } else {
        DBG( String("[LoraDevHW] LoRaWAN send failed:") + gRadioLibErrStr(state));
    }
}

void LoraDevHW::onUpdate()
{
}

void LoraDevHW::onReceive(uint8_t *data, uint32_t size)
{
    DBG("[LoraDevHW] Collar::onReceive() Not implemented!");
}

float LoraDevHW::rssi()
{
    return mRadio.getRSSI();
}

float LoraDevHW::snr()
{
    return mRadio.getSNR();
}

String LoraDevHW::addrHex()
{
    uint8_t ba[LORA_ADDR_LEN] = {0};
    ba[3] = (uint8_t)  (mAddr & 0x000000FF);
    ba[2] = (uint8_t) ((mAddr & 0x0000FF00) >> 8 );
    ba[1] = (uint8_t) ((mAddr & 0x00FF0000) >> 16);
    ba[0] = (uint8_t) ((mAddr & 0xFF000000) >> 24);
    return toHex(ba, LORA_ADDR_LEN);
}

String LoraDevHW::fportHex()
{
    return toHex(&mFPort, LORA_FPORT_LEN);
}

void LoraDevHW::setFPortHex(const char *fp)
{
    fromHex(fp, &mFPort, LORA_FPORT_HEX_LEN );
}

void LoraDevHW::setAddrHex(const char *addr)
{
    uint8_t dst[LORA_ADDR_LEN] = {0};
    fromHex(addr, dst, LORA_ADDR_HEX_LEN);
    mAddr = (uint32_t)(dst[3]) | ((uint32_t)(dst[2]) << 8) | ((uint32_t)(dst[1]) << 16) | ((uint32_t)(dst[0]) << 24);
}



void LoraDevHW::setAKey(const uint8_t *key)
{
    memcpy(mAKey, key, LORA_KEY_LEN);
}

void LoraDevHW::setNKey(const uint8_t *key)
{
    memcpy(mNKey, key, LORA_KEY_LEN);
}



String LoraDevHW::toHex(uint8_t* a, int len)
{
    String result;
    char hex[4] = {0};
    for( int i = 0; i < len; i ++) {
        if( a[i]  < 16) {
            std::sprintf(hex, "0%x", a[i]);
        } else {
            std::sprintf(hex, "%x", a[i]);
        }
        result += hex;
    }

    return result;
}



void LoraDevHW::fromHex(const char *src, uint8_t *dst, int srcLen)
{
    auto x2b4 = [&](char b) {
        if( b >= '0' && b <= '9') b -= '0';
        else
            if( b >= 'a' && b <= 'z') b = b - 'a' + 10;
            else
                if( b >= 'A' && b <= 'Z') b = b - 'A' + 10;
        return (uint8_t)b;
    };

    auto x2b = [&](const char* b) {
        uint8_t bl = x2b4(b[0]);
        uint8_t bh = x2b4(b[1]);
        return bh | (bl << 4);
    };

    int dstLen = srcLen / 2;
    for( int i = 0; i < dstLen; i ++ ) {
        dst[i] = x2b( &src[i * 2] );
    }
}



// #ifndef  ONPC
    LoraDevHW gLDC;
// #endif

// #endif

