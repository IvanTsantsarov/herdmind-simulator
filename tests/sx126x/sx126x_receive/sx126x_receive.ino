/*
  RadioLib SX126x Receive with Interrupts Example

  Other modules from SX126x family can also be used.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// #define SETTINGS_RX1 1

// include the library
#include <RadioLib.h>

#define PIN_LED 35

// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9
// SX1262 radio = new Module(10, 2, 3, 9);

#define PIN_NSS   8
#define PIN_SCK   9
#define PIN_MOSI  10
#define PIN_MISO  11
#define PIN_DIO1  14
#define PIN_NRST  12
#define PIN_BUSY  13

SX1262 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);

int gAttempt = 0;
uint64_t gMsecLastMsg = 0;
uint64_t gMsecOneSecond = 0;
time_t tLast = 0;

void setup() {
    Serial.begin(115200);

    radio.tcxoVoltage = 1.8f;

    ConfigLoRa_t config;

    config.syncWord = 0x34;            // LoRaWAN public sync word
    radio.invertIQ(true);
    radio.setCRC(0);
    config.codingRate = 5;             // 4/5
    config.preambleLength = 8;
    config.bandwidth = 125.0;

#ifdef SETTINGS_RX1
    config.frequency = 868.5;       // Gateway's RX2 downlink frequency
    config.spreadingFactor = 9;
#else
    config.frequency = 869.525;       // Gateway's RX2 downlink frequency
    config.spreadingFactor = 12;
#endif
    

    int state = radio.begin(config);
    Serial.println("*******************************");
    Serial.printf("*** BEGIN: state = %d\n", state);
    Serial.println("*******************************");
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("Failed!");
        while (true) delay(1000);
    }

    // In setup(), after radio.begin(config):
    radio.setPacketReceivedAction(setFlag);


    state = radio.setCRC(0);           // Downlink has no PHY CRC
    Serial.printf("setCRC: %d\n", state);

    state = radio.invertIQ(true);      // Downlink IQ polarity
    Serial.printf("invertIQ: %d\n", state);

    state = radio.startReceive();
    Serial.printf("startReceive: %d\n", state);

    pinMode(PIN_LED, OUTPUT);
}

volatile bool receivedFlag = false;

void setFlag() {
    receivedFlag = true;
}


void loop() {
    
    uint64_t msec = millis();
    uint64_t msecDelta = msec - gMsecOneSecond;


    if( msecDelta > 1000 ) {
        Serial.printf(".");
        gMsecOneSecond = msec;
        digitalWrite(PIN_LED, HIGH);
        delay(10);
        digitalWrite(PIN_LED, LOW);
    }

    if (!receivedFlag) {
        return;
    }

    receivedFlag = false;

    uint8_t data[255] = {0};
    size_t dataLen = radio.getPacketLength();

    int16_t state = radio.readData(data, dataLen);

    msecDelta = msec - gMsecLastMsg;


    gMsecLastMsg = msec;
    Serial.println(String("\n[") + msec + "," + msecDelta + "]" + " length:" +  dataLen);

    if (state == RADIOLIB_ERR_NONE) {
        bool isString = true;
        for (size_t i = 0; i < dataLen; i++) {
            Serial.printf("%02X ", data[i]);
            if( isString && !isalnum(data[0]) ) {
                isString = false;
            }
        }
        Serial.println();
                
        if( isString ) {
            data[dataLen] = 0;
            Serial.println((const char*)data);
            // Serial.printf("RSSI: %.1f dBm\n", radio.getRSSI());
        }

        digitalWrite(PIN_LED, HIGH);
        delay(200);
        digitalWrite(PIN_LED, LOW);
    }

    state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("startReceive failed: %d\n", state);
    }
}