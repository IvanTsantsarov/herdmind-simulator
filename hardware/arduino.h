#pragma once

#ifdef ONPC
    #include "tools.h"
#else
    #include <Arduino.h>
    #include <LoRaWan_APP.h>
    #include <HT_TinyGPS++.h>
    #include <esp_mac.h> // Required for ESP32 MAC/ChipID functions
#endif
