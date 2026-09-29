#include "loradev_bolus.h"

#ifndef ONPC

#include "esp_mac.h" // Required for ESP32 MAC/ChipID functions

LoraDev::LoraDev() {

    uint8_t mac[6];

    // Get the base MAC address of the ESP32 (6 bytes unique)
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
}


void LoraDev::onSetup() {
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

    char hex[3] = {0};
    for( int i = 0; i < 8; i ++) {
        itoa(hex, mEui[7-i], 16);
        mEuiHex += hex;
    }
}

#endif