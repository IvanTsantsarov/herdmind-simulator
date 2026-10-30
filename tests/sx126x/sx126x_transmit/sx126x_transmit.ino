/*
  LoRaRawTestTransmitter.ino
  Raw LoRa test transmitter for a matching SX1262 receiver.

  PHY settings:
    869.525 MHz, BW 125 kHz, SF12, CR 4/5
    Sync word 0x34, inverted IQ, no PHY CRC

  This sends plain test text, NOT a LoRaWAN frame.
*/

#include <RadioLib.h>

#define PIN_NSS   8
#define PIN_SCK   9
#define PIN_MOSI  10
#define PIN_MISO  11
#define PIN_DIO1  14
#define PIN_NRST  12
#define PIN_BUSY  13

SX1262 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);

uint32_t packetCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Heltec V4 TCXO configuration.
  radio.tcxoVoltage = 1.8f;

  ConfigLoRa_t config;
  config.frequency = 869.525;
  config.bandwidth = 125.0;
  config.spreadingFactor = 12;
  config.codingRate = 5;       // 4/5
  config.syncWord = 0x34;      // LoRaWAN public sync word
  config.preambleLength = 8;

  Serial.println(F("[TX] Initializing SX1262..."));
  int16_t state = radio.begin(config);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[TX] begin() failed: "));
    Serial.println(state);
    while (true) delay(1000);
  }

  state = radio.setCRC(0);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[TX] setCRC(0) failed: "));
    Serial.println(state);
    while (true) delay(1000);
  }

  state = radio.invertIQ(true);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[TX] invertIQ(true) failed: "));
    Serial.println(state);
    while (true) delay(1000);
  }

  Serial.println(F("[TX] Ready; sending one short test packet every 10 seconds."));
}

void loop() {
  char payload[64];
  snprintf(payload, sizeof(payload), "Zdrasti, Vankaaaa!!! -%lu", (unsigned long)packetCounter++);

  Serial.print(F("[TX] Sending: "));
  Serial.println(payload);

  int16_t state = radio.transmit((uint8_t*)payload, strlen(payload));
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("[TX] transmit() success"));
  } else {
    Serial.print(F("[TX] transmit() failed: "));
    Serial.println(state);
  }

  // SF12 packets have relatively long airtime; keep the test rate low.
  delay(10000);
}
