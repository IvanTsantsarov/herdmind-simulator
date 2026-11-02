#include "../arduino.h"

#define RADIO_LIB_ERRCODE_CASE(__err__) case __err__: return String(#__err__) + "(" + __err__ + ")";

String gRadioLibErrStr(int16_t errorCode) {
    switch(errorCode) {

#ifndef ONPC
        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_NONE)

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_UNKNOWN                                   )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_CHIP_NOT_FOUND                            )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_MEMORY_ALLOCATION_FAILED                  )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_PACKET_TOO_LONG                           )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_TX_TIMEOUT                                )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_RX_TIMEOUT                                )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_CRC_MISMATCH                              )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_BANDWIDTH                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_SPREADING_FACTOR                  )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_CODING_RATE                       )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_BIT_RANGE                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_FREQUENCY                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_OUTPUT_POWER                      )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_PREAMBLE_DETECTED                             )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_CHANNEL_FREE                                  )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_SPI_WRITE_FAILED                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_CURRENT_LIMIT                     )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH                   )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_GAIN                              )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_WRONG_MODEM                               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_NUM_SAMPLES                       )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_RSSI_OFFSET                       )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_ENCODING                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_LORA_HEADER_DAMAGED                       )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_UNSUPPORTED                               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_DIO_PIN                           )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_RSSI_THRESHOLD                    )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_NULL_POINTER                              )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_IRQ                               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_PACKET_TOO_SHORT                          )

        // RF69-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_BIT_RATE                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_FREQUENCY_DEVIATION               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_BIT_RATE_BW_RATIO                 )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_RX_BANDWIDTH                      )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_SYNC_WORD                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_DATA_SHAPING                      )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_MODULATION                        )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_OOK_RSSI_PEAK_TYPE                )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_BIT_RATE_TOLERANCE_VALUE          )

        // APRS status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_SYMBOL                            )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_MIC_E_TELEMETRY                   )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_MIC_E_TELEMETRY_LENGTH            )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_MIC_E_TELEMETRY_STATUS                    )

        // SSDV status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_SSDV_MODE                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_IMAGE_SIZE                        )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_IMAGE_QUALITY                     )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_SUBSAMPLING                       )
        // RTTY status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_RTTY_SHIFT                        )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_UNSUPPORTED_ENCODING                      )
        // nRF24-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_DATA_RATE                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_ADDRESS_WIDTH                     )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_PIPE_NUMBER                       )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_ACK_NOT_RECEIVED                          )

        // CC1101-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_NUM_BROAD_ADDRS                   )

        // SX126x-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_CRC_CONFIGURATION                 )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_LORA_DETECTED                                 )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_TCXO_VOLTAGE                      )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_MODULATION_PARAMETERS             )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_SPI_CMD_TIMEOUT                           )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_SPI_CMD_INVALID                           )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_SPI_CMD_FAILED                            )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_SLEEP_PERIOD                      )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_RX_PERIOD                         )

        // AX.25-specific status codes
        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_CALLSIGN                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_NUM_REPEATERS                     )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_REPEATER_CALLSIGN                 )

        // SX128x-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_RANGING_TIMEOUT                           )

        // Pager-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_PAYLOAD                            )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_ADDRESS_NOT_FOUND                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_FUNCTION                           )

        // LoRaWAN-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_NETWORK_NOT_JOINED                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_DOWNLINK_MALFORMED                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_REVISION                           )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_PORT                               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_NO_RX_WINDOW                               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_NO_CHANNEL_AVAILABLE                       )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_CID                                )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_UPLINK_UNAVAILABLE                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_COMMAND_QUEUE_FULL                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_JOIN_NONCE_INVALID                         )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_MIC_MISMATCH                               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_MULTICAST_FCNT_INVALID                     )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_DWELL_TIME_EXCEEDED                        )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_CHECKSUM_MISMATCH                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_NO_JOIN_ACCEPT                             )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_LORAWAN_SESSION_RESTORED                       )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_LORAWAN_NEW_SESSION                            )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_NONCES_DISCARDED                           )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_SESSION_DISCARDED                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_MODE                               )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_MULTICAST_GROUP                    )

        // LR11x0-specific status codes

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_INVALID_WIFI_TYPE                          )

        RADIO_LIB_ERRCODE_CASE(RADIOLIB_ERR_GNSS_SUBFRAME_NOT_AVAILABLE                )

        RADIO_LIB_ERRCODE_CASE( RADIOLIB_ERR_GNSS_DEMOD_OFFSET                          )

        RADIO_LIB_ERRCODE_CASE( RADIOLIB_ERR_GNSS_SOLVER_OFFSET                         )

        // LR2021-specific status codes

        RADIO_LIB_ERRCODE_CASE( RADIOLIB_ERR_FRONTEND_CALIBRATION_FAILED                )

        RADIO_LIB_ERRCODE_CASE( RADIOLIB_ERR_INVALID_SIDE_DETECT                        )

        // ADS-B-specific status codes

        RADIO_LIB_ERRCODE_CASE( RADIOLIB_ERR_ADSB_INVALID_MSG_TYPE                      )
#endif
    default: return String("Unknown lora code:") + errorCode;
    }
}