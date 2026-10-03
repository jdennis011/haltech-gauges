#pragma once

#if HG_BOARD_M5DIAL
// M5Stack Dial (StampS3, ESP32-S3FN8, no PSRAM). The display, touch, encoder
// and the rest are handled by the M5Dial library; only the pins this firmware
// drives itself are listed.

// Port B (HY2.0-4P): white = GPIO1, yellow = GPIO2, plus 5 V and GND.
#define PIN_CAN_TX 1
#define PIN_CAN_RX 2

#else
// Waveshare ESP32-S3-Touch-AMOLED-1.75

// CO5300 AMOLED, QSPI
#define PIN_LCD_CS 12
#define PIN_LCD_SCLK 38
#define PIN_LCD_D0 4
#define PIN_LCD_D1 5
#define PIN_LCD_D2 6
#define PIN_LCD_D3 7
#define PIN_LCD_RST 39
#define PIN_LCD_TE 13

// Shared I2C bus: CST9217 touch, AXP2101 PMIC, TCA9554 expander, IMU, RTC, codec
#define PIN_I2C_SDA 15
#define PIN_I2C_SCL 14
#define PIN_TOUCH_INT 11
#define PIN_TOUCH_RST 40

// The only free GPIOs, on the rear 1x8 header. Waveshare's hardware reference
// and schematic disagree on which header position each one sits at; the
// bring-up firmware toggles them at different rates so they can be mapped.
#define PIN_CAN_TX 17
#define PIN_CAN_RX 18
#define PIN_SPARE 16
#endif
