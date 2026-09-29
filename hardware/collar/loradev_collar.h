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

    static String arrayToString(uint8_t* a, int len);
    static void setArray(uint8_t* src, uint8_t* dst, int len);
public:
    LoraDevCollar();
    String euiStr(){ return arrayToString(mEui, LORA_EUI_LEN); }

    const uint8_t* nkey(){ return mNKey; }
    String nkeyStr(){ return arrayToString(mNKey, LORA_KEY_LEN); }

    const uint8_t* akey(){ return mAKey; }
    String akeyStr(){ return arrayToString(mAKey, LORA_KEY_LEN); }

    void setEui(uint8_t* eui) { setArray(eui, mEui, LORA_EUI_LEN); }
    void setNKey(uint8_t* key) { setArray(key, mNKey, LORA_KEY_LEN); }
    void setAKey(uint8_t* key) { setArray(key, mAKey, LORA_KEY_LEN); }

    virtual void onSetup();
};
 // #endif
