#include "imu.h"

#include <stdio.h>

// The M5Stack Dial has no accelerometer: a gmeter on it sits at rest.
namespace imu {

void begin() {}
bool present() { return false; }
void poll(uint32_t) {}
Reading reading() { return Reading(); }
float lonRaw() { return 0; }
uint32_t msPerSecond() { return 0; }
bool calibrated() { return false; }
bool levelSet() { return false; }
bool forwardSet() { return false; }
bool setLevel() { return false; }
bool setForward() { return false; }
void clearCalibration() {}
void status(char* out, size_t len) { snprintf(out, len, "imu: none on this board"); }

}  // namespace imu
