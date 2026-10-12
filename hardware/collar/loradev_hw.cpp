#include "loradev_hw.h"

#include <cstdio>

// #ifndef ONPC

//namespace Heltec {
//};


//using namespace Heltec;

// OTAA para (not used)
uint8_t devEui[] = { 0x22, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x23 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
// ABP para
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

// LoraWan channelsmask, default channels 0-7
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
uint32_t license[4] = {0x21701168, 0xAAAEEC0A, 0xFDA9F9D2, 0x0110E29E};
// LoraWan region, select in arduino IDE tools
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

// LoraWan Class, Class A and Class C are supported
DeviceClass_t  loraWanClass = CLASS_A;

// the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

// OTAA or ABP
bool overTheAirActivation = true;

// ADR enable
bool loraWanAdr = true;
// Indicates if the node is sending confirmed or unconfirmed messages
bool isTxConfirmed = true;

// Application port
uint8_t appPort = 2;
/*!
  Number of trials to transmit the frame, if the LoRaMAC layer did not
  receive an acknowledgment. The MAC performs a datarate adaptation,
  according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
  to the following table:

  Transmission nb | Data Rate
  ----------------|-----------
  1 (first)       | DR
  2               | DR
  3               | max(DR-1,0)
  4               | max(DR-1,0)
  5               | max(DR-2,0)
  6               | max(DR-2,0)
  7               | max(DR-3,0)
  8               | max(DR-3,0)

  Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
  the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;

LoraDevHW::LoraDevHW() {

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

    memcpy( nwkSKey, mNKey, LORA_KEY_LEN );
    memcpy( appSKey,mAKey, LORA_KEY_LEN );
    devAddr =  mAddr;

    LoRaWAN.init(loraWanClass, loraWanRegion);
}

void LoraDevHW::sendPackage(void *data, int len)
{
    const char* charPtr = ((char*)data);
    for( appDataSize = 0; appDataSize < len; appDataSize ++) {
        appData[appDataSize] = charPtr[appDataSize++];
    }
}

void LoraDevHW::onUpdate()
{
    switch ( deviceState )
    {
        case DEVICE_STATE_INIT:
        {
    #if(LORAWAN_DEVEUI_AUTO)
            LoRaWAN.generateDeveuiByChipID();
    #endif
            LoRaWAN.init(loraWanClass,loraWanRegion);
            //both set join DR and DR when ADR off
            LoRaWAN.setDefaultDR(3);
            break;
        }
        case DEVICE_STATE_JOIN:
        {
            LoRaWAN.join();
            break;
        }
        case DEVICE_STATE_SEND:
        {
            // prepareTxFrame( appPort );
            LoRaWAN.send();
            deviceState = DEVICE_STATE_CYCLE;
            break;
        }
        case DEVICE_STATE_CYCLE:
        {
            txDutyCycleTime = appTxDutyCycle;
            LoRaWAN.cycle(txDutyCycleTime);
            deviceState = DEVICE_STATE_SLEEP;
            break;
        }
        case DEVICE_STATE_SLEEP:
        {

            LoRaWAN.sleep(loraWanClass);
            break;
        }
        default:
        {
            deviceState = DEVICE_STATE_INIT;

            break;
        }
    }
}


// #ifndef  ONPC
    LoraDevHW gLDC;
// #endif

// #endif