#ifndef DEFINES_H
#define DEFINES_H

#define COLLAR_VERSION "0.81"
#define DBG(__str__) Serial.println(__str__); Serial.flush();

#define LORA_FPORT 2

#define LORA_FPORT_LEN 1
#define LORA_FPORT_HEX_LEN 2
#define LORA_EUI_LEN 8
#define LORA_EUI_HEX_LEN 16
#define LORA_KEY_LEN 16
#define LORA_KEY_HEX_LEN 32
#define LORA_ADDR_LEN 4
#define LORA_ADDR_HEX_LEN 8
#define LORA_NAME_MAX_LEN 32

#define SERIAL_CMD_SIZE 7
#define SERIAL_CMD_MIN_SIZE 2

#endif // DEFINES_H
