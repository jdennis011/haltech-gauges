#pragma once

// A console on the gauge's USB serial port, for the bench: paste a config as
// JSON to show it, switch faces, replay the intro, set the clock. Type ? for
// the list. Everything here can also be done by the hub over CAN; this is for
// a gauge on a USB lead with no hub, as in the first bring-up.
namespace serial_console {

void begin();
// From loop(): reads what has arrived and acts on complete lines.
void poll();

}  // namespace serial_console
