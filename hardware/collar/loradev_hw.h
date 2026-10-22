#pragma once

// #ifndef ONPC
#include "../arduino.h"
#include "defines.h"

//namespace Heltec
//{
/*
*/
//};

#define LORAWAN_APP_DATA_MAX_SIZE 64

class LoraDevHW
{
    uint8_t mFPort = LORA_FPORT;
    uint8_t mEui[LORA_EUI_LEN];
    uint8_t mNKey[LORA_KEY_LEN];
    uint8_t mAKey[LORA_KEY_LEN];
    uint32_t mAddr;
    uint8_t mBufferUp[LORAWAN_APP_DATA_MAX_SIZE];
    size_t mBufferUpLen = 0;
    uint8_t mBufferDown[LORAWAN_APP_DATA_MAX_SIZE];
    size_t mBufferDownLen = 0;

    static void fromHex(const char* src, uint8_t *dst, int srcLen);
    static String toHex(uint8_t* a, int len);

    void upplayParamsABP();

    bool mIsSent = false;

    SX1262 mRadio = nullptr;
    LoRaWANNode* mNode = nullptr;

public:

    LoraDevHW();

    inline uint8_t fport(){ return mFPort; }

    String euiHex(){ return toHex(mEui, LORA_EUI_LEN); }

    const uint8_t* nkey(){ return mNKey; }
    String nkeyHex(){ return toHex(mNKey, LORA_KEY_LEN); }

    const uint8_t* akey(){ return mAKey; }
    String akeyHex(){ return toHex(mAKey, LORA_KEY_LEN); }

    String addrHex();
    String fportHex();

    void setFPortHex(const char* fp);
    void setEuiHex(const char* eui) { fromHex(eui, mEui, LORA_EUI_HEX_LEN); }
    void setNKeyHex(const char* key) { fromHex(key, mNKey, LORA_KEY_HEX_LEN); }
    void setAKeyHex(const char* key) { fromHex(key, mAKey, LORA_KEY_HEX_LEN); }
    void setAddrHex(const char* addr);


    void setAKey(const uint8_t* key);
    void setNKey(const uint8_t* key);
    void setAddr(uint32_t a) { mAddr = a; }
    void setFPort(uint8_t fp) { mFPort = fp; }

    void sendPackage(void* data, size_t len);

    virtual void onSetup();
    virtual void onUpdate();
    virtual void onReceive(uint8_t* data, uint32_t size); // data receiving
};
 // #endif
