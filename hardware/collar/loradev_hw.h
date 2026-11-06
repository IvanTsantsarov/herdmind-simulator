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
    static RTC_DATA_ATTR uint32_t mBootCount;
    static RTC_DATA_ATTR uint8_t mFPort;
    static RTC_DATA_ATTR uint8_t mEui[LORA_EUI_LEN];
    static RTC_DATA_ATTR uint8_t mNKey[LORA_KEY_LEN];
    static RTC_DATA_ATTR uint8_t mAKey[LORA_KEY_LEN];
    static RTC_DATA_ATTR uint32_t mAddr;
    static RTC_DATA_ATTR uint8_t mLoraSession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
    static RTC_DATA_ATTR uint8_t mLoraNonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
    static RTC_DATA_ATTR bool mLoraSessionValid;

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

    virtual float rssi();
    virtual float snr();
    static inline bool isWakingUp(){ return LoraDevHW::mBootCount > 0; };
    static inline int bootCount(){ return LoraDevHW::mBootCount; }
    static void incBootCount(){ LoraDevHW::mBootCount++; }

    virtual bool sleep(uint64_t interval, int pinBtnWake);
};
 // #endif
