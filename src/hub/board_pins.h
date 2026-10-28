#pragma once

#include <sdkconfig.h>

#if CONFIG_IDF_TARGET_ESP32C3
// ESP32-C3-DevKitM-1, development only: one CAN controller, so just the gauge
// bus. Avoids the strapping pins (2, 8, 9), USB (18, 19) and UART0 (20, 21).
#define PIN_GAUGE_CAN_TX 4
#define PIN_GAUGE_CAN_RX 5
#define PIN_VBAT_SENSE 0
// Steering wheel outputs on the bench: the board's LED stands in for the horn relay.
#define PIN_HORN 7
#define PIN_SWC_0 6
#define PIN_SWC_1 10
#define PIN_SWC_2 1
#define PIN_SWC_3 3
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

// Steering wheel module outputs (docs/steering-wheel.md, electrical-spec.md 6.3),
// on spare pins: the vehicle CAN transceiver to the Nexus's CAN 2, its silent
// pin, the horn relay drive (pulled down on the carrier), the wired-remote outputs.
#if HG_FG_ON_GAUGE_PORT
// Test build (env hub_p4_fgtest): the vehicle CAN on the gauge port's pins and
// transceiver, the gauge CAN bus not started (docs/steering-wheel.md).
#define PIN_FG_CAN_TX PIN_GAUGE_CAN_TX
#define PIN_FG_CAN_RX PIN_GAUGE_CAN_RX
#else
#define PIN_FG_CAN_TX 26
#define PIN_FG_CAN_RX 27
#endif
#define PIN_FG_CAN_SILENT 52
#define PIN_HORN 49
#define PIN_SWC_0 2
#define PIN_SWC_1 3
#define PIN_SWC_2 4
#define PIN_SWC_3 5
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

// Steering wheel outputs (no vehicle CAN port: two controllers only)
#define PIN_HORN 11
#define PIN_SWC_0 0
#define PIN_SWC_1 1
#define PIN_SWC_2 3
#define PIN_SWC_3 10

// 12 V supply through a divider, for logging cranking dips (ADC1 channel 2)
#define PIN_VBAT_SENSE 2
#endif
