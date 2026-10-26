#pragma once

// The steering wheel module on a Lolin C3 Mini (docs/steering-wheel.md).
// Strapping pins 2, 8 and 9 and the USB pins 18 and 19 are left alone, bar
// GPIO2 as an optional fourth ladder line (a pull-up is its right boot state).
#define PIN_CAN_TX 4
#define PIN_CAN_RX 5
// Ladder lines, ADC1: each has a pull-up to 3V3 on the board and reads the
// wheel's resistor ladder through a series resistor.
#define PIN_LINE_0 0
#define PIN_LINE_1 1
#define PIN_LINE_2 3
#define PIN_LINE_3 2
#define PIN_LED 7          // the board's LED: slow blink no hub, on with a hub, fast blink identify
#define PIN_BOOT_BUTTON 9  // held three seconds at power-up: the calibration is cleared
