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
// Rear header, measured with the meter on 2026-10-29 (Waveshare's hardware
// reference has positions 6 and 7 the other way round): position 6 = GPIO16,
// position 7 = GPIO17, position 8 showed no connection. The adapter puts the
// transceiver's D input on 6 and its R output on 7.
#define PIN_CAN_TX 16
#define PIN_CAN_RX 17
#define PIN_SPARE 18  // would be position 8; not found on the board, unused

// ES8311 codec (0x18 on the shared I2C bus) and its speaker amplifier (docs/gauge-audio.md)
#define PIN_I2S_MCLK 42
#define PIN_I2S_BCLK 9
#define PIN_I2S_WS 45
#define PIN_I2S_DOUT 8   // to the codec's DSDIN
#define PIN_I2S_DIN 10   // from the codec's ASDOUT (microphone path, unused)
#define PIN_AUDIO_AMP 46 // amplifier enable, high = on
#endif
