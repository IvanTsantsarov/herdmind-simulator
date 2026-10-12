#pragma once

// #ifndef ONPC
#include "../arduino.h"
#include "defines.h"

//namespace Heltec
//{
/*
*/
//};

class LoraDevHW
{
    uint8_t mEui[LORA_EUI_LEN];
    uint8_t mNKey[LORA_KEY_LEN];
    uint8_t mAKey[LORA_KEY_LEN];
    uint32_t mAddr;
    uint8_t mBuffer[LORAWAN_APP_DATA_MAX_SIZE];
    uint8_t mBufferLen = 0;

    static void fromHex(const char* src, uint8_t *dst, int srcLen);
    static String toHex(uint8_t* a, int len);

    enum struct State {
        Init = 0,
        Join,
        Send,
        Cycle,
        Sleep
    };

    State mState = State::Init;

    void upplayParamsABP();

public:

    LoraDevHW();
    String euiHex(){ return toHex(mEui, LORA_EUI_LEN); }

    const uint8_t* nkey(){ return mNKey; }
    String nkeyHex(){ return toHex(mNKey, LORA_KEY_LEN); }

    const uint8_t* akey(){ return mAKey; }
    String akeyHex(){ return toHex(mAKey, LORA_KEY_LEN); }

    String addrHex();

    void setEuiHex(const char* eui) { fromHex(eui, mEui, LORA_EUI_HEX_LEN); }
    void setNKeyHex(const char* key) { fromHex(key, mNKey, LORA_KEY_HEX_LEN); }
    void setAKeyHex(const char* key) { fromHex(key, mAKey, LORA_KEY_HEX_LEN); }
    void setAddrHex(const char* addr);


    void setAKey(const uint8_t* key);
    void setNKey(const uint8_t* key);
    void setAddr(uint32_t a) { mAddr = a; }

    void sendPackage(void* data, int len);

    virtual void onSetup();
    virtual void onUpdate();
};
 // #endif
