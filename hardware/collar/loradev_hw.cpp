#include "loradev_hw.h"

#include <cstdio>

// #ifndef ONPC

#ifndef ONPC
    #include "esp_mac.h" // Required for ESP32 MAC/ChipID functions
#endif

LoraDevHW::LoraDevHW() {

    uint8_t mac[6];

    // Get the base MAC address of the ESP32 (6 bytes unique)
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
}

String LoraDevHW::addrHex()
{
    uint8_t ba[LORA_ADDR_LEN];
    ba[0] = (uint8_t) mAddr & 0x000000FF;
    ba[1] = (uint8_t) (mAddr & 0x0000FF00) >> 8;
    ba[2] = (uint8_t) (mAddr & 0x00FF0000) >> 16;
    ba[3] = (uint8_t) (mAddr & 0xFF000000) >> 24;
    return toHex(ba, LORA_ADDR_LEN);
}

void LoraDevHW::setAddrHex(const char *addr)
{
    uint8_t dst[LORA_ADDR_LEN];
    fromHex(addr, dst, LORA_ADDR_HEX_LEN);
    mAddr = (uint32_t)(dst[0]) | ((uint32_t)(dst[1]) << 8) | ((uint32_t)(dst[2]) << 16) | ((uint32_t)(dst[3]) << 24);
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
    char hex[3] = {0};
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

void LoraDevHW::onSetup() {
    uint8_t mac[6];

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

}

#ifndef  ONPC
    LoraDevHW gLDC;
#endif

// #endif