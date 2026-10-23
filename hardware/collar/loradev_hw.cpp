#include "loradev_hw.h"

#include <cstdio>

#define PIN_NSS   8
#define PIN_SCK   9
#define PIN_MOSI  10
#define PIN_MISO  11
#define PIN_DIO1  14
#define PIN_NRST  12
#define PIN_BUSY  13


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


LoraDevHW::LoraDevHW() {
    // 1. Map the exact Heltec WiFi LoRa 32 V4.3 internal hardware pins
    // Pins for V4.3: NSS=8, DIO1=14, NRST=12, BUSY=13
    mRadio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);

    // 2. Create the LoRaWAN Node Instance
    mNode = new LoRaWANNode(&mRadio, &EU868); // Automatically sets up EU868 frequencies
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


    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_NSS);

    ConfigLoRa_t config{};
    config.frequency = 868; // The frequency here does not matter, as it will get changed by LoRaWAN anyway
    //radio.tcxoVoltage = 1.6; // Some radio modules like SX126x often come with TCXO

    Serial.println(String("[LoraDevHW] === Setup start ==="));
    Serial.println(String("[LoraDevHW] Addr:") + addrHex() + ", fport:" + fportHex());
    Serial.println(String("[LoraDevHW] NKey:") + nkeyHex());
    Serial.println(String("[LoraDevHW] AKey:") + akeyHex());
    Serial.println(String("[LoraDevHW] === Setup end ==="));


    int state = mRadio.begin(config);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("[LoraDevHW] ERROR:Lora init failed!");
        return;
    }

    // Pull in the ABP profiles natively
    state = mNode->beginABP(mAddr, nullptr, nullptr, mNKey, mAKey);

    if (state != RADIOLIB_ERR_NONE) {
        Serial.println( String("[LoraDevHW] ERROR:beginABP failed: ") + state);
        return;
    }

    state = mNode->activateABP();

    if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
        Serial.println("[LoraDevHW] ABP activated: new session");
    } else if (state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
        Serial.println("[LoraDevHW] ABP activated: session restored");
    } else if (state != RADIOLIB_ERR_NONE) {
        Serial.println(String("[LoraDevHW] ERROR: activateABP failed:") + state);
        return;
    }
}

void LoraDevHW::sendPackage(void *data, size_t len)
{
    const char* charPtr = ((char*)data);
    // memcpy(appData, data, static_cast<size_t>(len));
    // appDataSize = static_cast<uint8_t>(len);
    for( mBufferUpLen = 0; mBufferUpLen < len; mBufferUpLen ++) {
        mBufferUp[mBufferUpLen] = charPtr[mBufferUpLen];
    }

    // Fires an unconfirmed uplink packet directly on specified fport
    // int state = mNode->sendReceive( mBufferUp, mBufferUpLen, mFPort, false);
    int state = mNode->sendReceive( mBufferUp, mBufferUpLen, mFPort,
                                   mBufferDown, &mBufferDownLen,
                                   false);

    DBG(String("[LoraDevHW] Sending:") + toHex(mBufferUp, mBufferUpLen) + "on port:" + mFPort);

    if (state >= RADIOLIB_ERR_NONE) {
        DBG( "[LoraDevHW] Uplink done");

        if (state > 0) {
            DBG("[LoraDevHW] Downlink received");
            DBG( toHex(mBufferDown, mBufferDownLen) );
            onReceive(mBufferDown, mBufferDownLen);
        } else {
            Serial.println("[LoraDevHW] No downlink received");
        }
    } else {
        Serial.println( String("[LoraDevHW] LoRaWAN send failed:") + state);
    }

}

void LoraDevHW::onUpdate()
{

}

void LoraDevHW::onReceive(uint8_t *data, uint32_t size)
{
    Serial.println("[LoraDevHW] Collar::onReceive() Not implemented!");
}


// #ifndef  ONPC
    LoraDevHW gLDC;
// #endif

// #endif