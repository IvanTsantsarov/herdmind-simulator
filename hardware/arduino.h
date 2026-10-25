#pragma once

#include "lora.h"

#ifdef ONPC
    #include "tools.h"
#else
    #include <Arduino.h>

#if USING_LORA == LORA_RADIOLIB
    #include <RadioLib.h>
#elif USING_LORA == LORA_HELTEC
    #include <LoRaWan_APP.h>
#endif

    #include <TinyGPS++.h>
    #include <esp_mac.h> // Required for ESP32 MAC/ChipID functions
#endif
