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

#define PIN_VFEM 7
#define PIN_CSD 2
#define PIN_LORA_TX 5

extern String gRadioLibErrStr(int16_t errorCode);

// Msec to wait additionaly for the RX window
#define LORAWAN_RX_WINDOW 250

RTC_DATA_ATTR uint32_t LoraDevHW::mBootCount = 0;
RTC_DATA_ATTR uint8_t LoraDevHW::mFPort = LORA_FPORT;
RTC_DATA_ATTR uint8_t LoraDevHW::mEui[LORA_EUI_LEN] = {0};
RTC_DATA_ATTR uint8_t LoraDevHW::mNKey[LORA_KEY_LEN] = {0};
RTC_DATA_ATTR uint8_t LoraDevHW::mAKey[LORA_KEY_LEN] = {0};
RTC_DATA_ATTR uint32_t LoraDevHW::mAddr = 0;

RTC_DATA_ATTR uint8_t LoraDevHW::mLoraSession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE] = {0};
RTC_DATA_ATTR uint8_t LoraDevHW::mLoraNonces[RADIOLIB_LORAWAN_NONCES_BUF_SIZE] = {0};
RTC_DATA_ATTR bool LoraDevHW::mLoraSessionValid = false;
RTC_DATA_ATTR bool LoraDevHW::mIsDeepSleepEnabled = false;

LoraDevHW::LoraDevHW() {
    // Only one instance is allowed of this type
    DBG("[LoraDevHD] Checking for single instance...");
    // assert(0 == mAddr);
}

String wakeup_reason_string(esp_sleep_wakeup_cause_t wakeup_reason){

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch(wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_UNDEFINED: return String("exiting sleep (e.g., standard power-on reset, manual reset button, or a serial monitor connection toggling DTR/RTS).");
    case ESP_SLEEP_WAKEUP_ALL: return String("internally to disable all wakeup sources.");
    case ESP_SLEEP_WAKEUP_EXT0 : return String("externa EXT0(RTC_IO)");
    case ESP_SLEEP_WAKEUP_EXT1 : return String("external EXT1(RTC_CNTL)");
    case ESP_SLEEP_WAKEUP_TIMER : return String("RTC timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : return String("touchpad");
    case ESP_SLEEP_WAKEUP_ULP : return String("ULP program");
    case ESP_SLEEP_WAKEUP_GPIO : return String("standard GPIO interrupt (Supported primarily during light sleep on ESP32, S2, and S3 chips).");
    case ESP_SLEEP_WAKEUP_UART : return String("UART data reception (light sleep only).");
    case ESP_SLEEP_WAKEUP_WIFI : return String("Wi-Fi MAC layer (light sleep only).");
    case ESP_SLEEP_WAKEUP_COCPU : return String("RISC-V ULP co-processor command.");
    default : return String("deep sleep") +  wakeup_reason;
    }

    return String("No reason");
}

bool LoraDevHW::isWakeBtn()
{
    return ESP_SLEEP_WAKEUP_EXT1 == mWakeUpCause;
}

bool LoraDevHW::isWakeTimer()
{
    return ESP_SLEEP_WAKEUP_TIMER == mWakeUpCause;
}



String reset_reason_string(int reason) {

    switch(reason) {
    case ESP_RST_UNKNOWN: return String("cannot be determined.");
    case ESP_RST_POWERON: return String("hardware power-on event (Vbat plugged in, power cycled");
    case ESP_RST_EXT: return String("physical external reset pin (CHIP_PU / EN pin)");
    case ESP_RST_SW: return String("programmatically via esp_restart()");
    case ESP_RST_PANIC: return String("core exception, crash, or panic");
    case ESP_RST_INT_WDT: return String("Interrupt Watchdog Timer (usually code blocking interrupts too long)");
    case ESP_RST_TASK_WDT: return String("Task Watchdog Timer (a task failed to feed the watchdog)");
    case ESP_RST_WDT: return String("system watchdogs");
    case ESP_RST_DEEPSLEEP: return String("exiting deep sleep mode");
    case ESP_RST_BROWNOUT: return String("drop in supply voltage (unstable power)");
    case ESP_RST_SDIO: return String("SDIO interface");
    }

    return String("Unknown");
}

void LoraDevHW::onSetup() {
    uint8_t mac[6] = {0};

    if( isWakingUp() ) {
        DBG("[LoraDevHW] --- Wake up ---");
        mWakeUpCause = esp_sleep_get_wakeup_cause();
        wakeup();
    }else {
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
    };

    mResetReason = esp_reset_reason();

    DBG(String("[LoraDevHW] Boot count:") + bootCount());
    DBG(String("[LoraDevHW] Addr:") + addrHex() + ", fport:" + fportHex());
    DBG(String("[LoraDevHW] NKey:") + nkeyHex());
    DBG(String("[LoraDevHW] AKey:") + akeyHex());

    DBG(String("[LoraDevHW] Reset reason: ") + reset_reason_string(mResetReason));
    DBG(String("[LoraDevHW] Wake cause: ") + wakeup_reason_string(mWakeUpCause));


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
    ///

    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_NSS);

    // 1. Power ON the Heltec V4.3 Front-End Module (PA/LNA)
    pinMode(PIN_VFEM, OUTPUT);
    digitalWrite(PIN_VFEM, HIGH); // VFEM power ON

    pinMode(PIN_CSD, OUTPUT);
    digitalWrite(PIN_CSD, HIGH); // FEM_EN (CSD) ON

    // 2. Tell RadioLib to automatically toggle GPIO 5 HIGH during TX and LOW during RX
    mRadio.setRfSwitchPins(RADIOLIB_NC, 5);

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

    // Activate session
    if (mLoraSessionValid) {

        state = mNode->setBufferNonces(mLoraNonces);
        DBG(String("[LoraDevHW] setBufferNonces: ") + gRadioLibErrStr(state));

        if (state == RADIOLIB_ERR_NONE) {
            state = mNode->setBufferSession(mLoraSession);
            DBG(String("[LoraDevHW] setBufferSession: ") + gRadioLibErrStr(state));
        }

        if (state == RADIOLIB_ERR_NONE) {
            state = mNode->activateABP();
        }else {
            DBG(String("[LoraDevHW] ERROR: setBufferSession failed: ") + gRadioLibErrStr(state));
            // Fallback to fresh activation if configuration was corrupted
            state = mNode->activateABP();
        }
    } else {
        state = mNode->activateABP();
    }

    if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
        DBG("[LoraDevHW] ABP activated: new session");
    } else if (state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
        DBG("[LoraDevHW] ABP activated: session restored");
    } else if (state != RADIOLIB_ERR_NONE) {
        DBG(String("[LoraDevHW] ERROR: activateABP failed:") + gRadioLibErrStr(state) );
        return;
    }

    if( isWakingUp() ) {
        DBG(String("[LoraDevHW] -----------------"));
    }else {
        DBG(String("[LoraDevHW] === Setup end ==="));
    }

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

    uint8_t* currentSessionPtr = mNode->getBufferSession();
    uint8_t* currentNoncesPtr = mNode->getBufferNonces();

    if (currentSessionPtr && currentNoncesPtr) {
        // Copy RadioLib's memory configuration straight to the persistent RTC block
        memcpy(mLoraSession, currentSessionPtr, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
        memcpy(mLoraNonces, currentNoncesPtr, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
        mLoraSessionValid = true;
        DBG("[LoraDevHW] Current LoRaWAN session serialized to RTC RAM.");
    } else {
        DBG("[LoraDevHW] Error: Could not fetch active buffers session and nonces from RadioLib.");
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

bool LoraDevHW::prepareToSleep(uint64_t msec, int pinBtnWake)
{
    if( !mIsDeepSleepEnabled ) {
        return true;
    }

    // Increment boot count
    LoraDevHW::incBootCount();

    mRadio.sleep(true);
    delay(50);

    // 2. Shut down the Front-End Module (FEM)
    digitalWrite(PIN_LORA_TX, LOW);  // TX/RX switch LOW
    digitalWrite(PIN_CSD, LOW);  // FEM_EN (CSD) OFF
    digitalWrite(PIN_VFEM, LOW);  // VFEM power supply OFF

    pinMode(PIN_LORA_TX, INPUT);
    pinMode(PIN_CSD, INPUT);
    pinMode(PIN_VFEM, INPUT);

    esp_err_t err = esp_sleep_enable_timer_wakeup(1000ULL * msec);
    DBG(String("[LoraDevHW] Timer config: ") + esp_err_to_name(err));

    err = esp_sleep_enable_ext1_wakeup(
        1ULL << pinBtnWake,
        ESP_EXT1_WAKEUP_ANY_LOW
        );
    DBG(String("[LoraDevHW] EXT1 config: ") + esp_err_to_name(err));

    return true;
}


bool LoraDevHW::sleep()
{
    if( !mIsDeepSleepEnabled ) {
        return true;
    }

    esp_deep_sleep_start();

    DBG("[LoraDevHW]Error: should not be here on a deep sleep!");

    return false;
}

void LoraDevHW::wakeup()
{
    // Power ON VFEM rail first and allow capacitors to stabilize
    pinMode(PIN_VFEM, OUTPUT);
    digitalWrite(PIN_VFEM, HIGH); // VFEM power ON
    delay(10);
    // Enable FEM chip (CSD)
    pinMode(PIN_CSD, OUTPUT);
    digitalWrite(PIN_CSD, HIGH); // FEM_EN (CSD) ON
    delay(2);
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

