#pragma once

#include <stdint.h>

#include "motion.h"

// The gauge board's accelerometer (docs/motion.md): the QMI8658 on the
// Waveshare 1.75 (the M5Stack Dial has none). Read from the UI loop, which
// owns the I2C bus along with the touch controller and the PMIC, so no lock
// is needed. The mounting calibration ("Set level", "Set forward" on the
// pull-down menu) is kept in the gauge's flash.
namespace imu {

// After board::begin().
void begin();
bool present();

// Reads the sensor when it is time to (every 20 ms). From the UI loop.
void poll(uint32_t nowMs);

// The car's acceleration in g, low-passed for the screen. valid is false
// without a sensor.
struct Reading {
    bool valid = false;
    float lat = 0, lon = 0, vert = 0;
};
Reading reading();
// The last longitudinal sample as it came, for the drag timer's launch.
float lonRaw();

// The mounting calibration: both captures done.
bool calibrated();
bool levelSet();
bool forwardSet();
// Captures the resting reading now (takes about 100 ms). False without a
// sensor or when the reading is not 1 g: the car was moving.
bool setLevel();
// Captures the push of a hard brake now. False when it is too weak.
bool setForward();
void clearCalibration();
// Time spent reading the sensor since the last call, for the fps line.
uint32_t msPerSecond();
// One line for the console.
void status(char* out, size_t len);

}  // namespace imu
