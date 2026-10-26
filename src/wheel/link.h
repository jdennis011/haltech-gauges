#pragma once

#include <stddef.h>
#include <stdint.h>

// The wheel module's place on the gauge chain (docs/steering-wheel.md): a
// GaugeNode that says it is a wheel, reporting the buttons as Input frames
// and taking the hub's commands, calibration and Wi-Fi fetch offers. Runs
// the CAN side and the button sampling on a task of its own.
namespace wheel_link {

void begin();
bool hubPresent();
uint32_t node();
uint32_t framesReceived();
// Identify was asked for: the LED shows it.
bool identifying();
// Set by a Reboot command or a finished firmware fetch; main() restarts.
void requestReboot();
bool rebootRequested();
// Frames sent so far, for the console.
uint32_t inputsSent();

}  // namespace wheel_link
