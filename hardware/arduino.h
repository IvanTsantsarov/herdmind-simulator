#pragma once

#include "lora.h"

#ifdef ONPC
    #include "tools.h"
#else
    #include <Arduino.h>

#if USING_LORA == LORA_RADIOLIB
    #include <RadioLib.h>
    #include <TinyGPS++.h>
#elif USING_LORA == LORA_HELTEC
    #include <LoRaWan_APP.h>
    #include <HT_TinyGPS++.h>
#endif


    #include <esp_mac.h> // Required for ESP32 MAC/ChipID functions
#endif
