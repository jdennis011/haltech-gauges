#include <Arduino.h>
#include <M5Dial.h>

#include "board.h"

namespace board {

namespace {
// The Dial's power latch: it goes back to sleep unless this is held high.
const int kHoldPin = 46;
}  // namespace

void begin() {
    pinMode(kHoldPin, OUTPUT);
    digitalWrite(kHoldPin, HIGH);

    auto cfg = M5.config();
    cfg.serial_baudrate = 115200;
    M5Dial.begin(cfg, true /* encoder */, false /* RFID */);
}

void update() { M5Dial.update(); }

uint16_t vbusMillivolts() { return 0; }

const char* name() { return "M5Stack Dial"; }

}  // namespace board
