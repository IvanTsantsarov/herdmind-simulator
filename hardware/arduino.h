#pragma once

#ifdef ONPC
    #include "tools.h"
#else
    #include <Arduino.h>
    #include <TinyGPS++.h>
    #include <esp_mac.h> // Required for ESP32 MAC/ChipID functions
    #include <RadioLib.h>
#endif
