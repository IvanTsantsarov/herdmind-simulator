#pragma once

// #ifndef ONPC
#include "../arduino.h"
#include "defines.h"

class LoraDevCollar
{
    uint8_t mEui[LORA_EUI_LEN];
    uint8_t mNKey[LORA_KEY_LEN];
    uint8_t mAKey[LORA_KEY_LEN];
    uint32_t mAddr;

    static void fromHex(const char* src, uint8_t *dst, int srcLen);
    static String toHex(uint8_t* a, int len);
public:
    LoraDevCollar();
    String euiStr(){ return toHex(mEui, LORA_EUI_LEN); }

    const uint8_t* nkey(){ return mNKey; }
    String nkeyStr(){ return toHex(mNKey, LORA_KEY_LEN); }

    const uint8_t* akey(){ return mAKey; }
    String akeyStr(){ return toHex(mAKey, LORA_KEY_LEN); }

    void setEui(const char* eui) { fromHex(eui, mEui, LORA_EUI_HEX_LEN); }
    void setNKey(const char* key) { fromHex(key, mNKey, LORA_KEY_HEX_LEN); }
    void setAKey(const char* key) { fromHex(key, mAKey, LORA_KEY_HEX_LEN); }

    virtual void onSetup();
};
 // #endif
