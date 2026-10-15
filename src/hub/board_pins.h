#pragma once

#include <sdkconfig.h>

#if CONFIG_IDF_TARGET_ESP32C3
// ESP32-C3-DevKitM-1, development only: one CAN controller, so just the gauge
// bus. Avoids the strapping pins (2, 8, 9), USB (18, 19) and UART0 (20, 21).
#define PIN_GAUGE_CAN_TX 4
#define PIN_GAUGE_CAN_RX 5
#define PIN_VBAT_SENSE 0
#elif CONFIG_IDF_TARGET_ESP32P4
// Waveshare ESP32-P4-WIFI6 on the P4 carrier (electrical-spec.md 5.3). GPIO14-19 and
// 54 are the on-board Wi-Fi module's SDIO link, 7/8 the board's I2C, 39-45 the microSD.

// Haltech bus transceiver
#define PIN_ECU_CAN_TX 21
#define PIN_ECU_CAN_RX 22
#define PIN_ECU_CAN_SILENT 23

// Gauge chain transceiver
#define PIN_GAUGE_CAN_TX 32
#define PIN_GAUGE_CAN_RX 33

// 12 V supply through a divider (ADC1 channel 4)
#define PIN_VBAT_SENSE 20
#else
// ESP32-C6-DevKitC-1. Avoids the strapping pins (4, 5, 8, 9, 15), USB (12, 13)
// and UART0 (16, 17).

// Haltech bus transceiver
#define PIN_ECU_CAN_TX 18
#define PIN_ECU_CAN_RX 19
// TJA1051T/3 S input: low = normal, high = silent (listens, cannot transmit or acknowledge)
#define PIN_ECU_CAN_SILENT 23

// Gauge chain transceiver
#define PIN_GAUGE_CAN_TX 20
#define PIN_GAUGE_CAN_RX 21

// 12 V supply through a divider, for logging cranking dips (ADC1 channel 2)
#define PIN_VBAT_SENSE 2
#endif
