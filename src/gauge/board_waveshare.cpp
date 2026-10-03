#include <Arduino.h>
#include <Wire.h>

#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>

#include "board.h"
#include "board_pins.h"

namespace board {

namespace {
XPowersPMU pmic;
bool pmicOk = false;
}  // namespace

void begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    pmicOk = pmic.begin(Wire, AXP2101_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL);
    if (!pmicOk) {
        Serial.println("PMIC start failed");
        return;
    }
    // The PMIC throttles its input current when the supply sags below this
    // limit. At the end of the gauge chain the 5 V rail arrives a few hundred
    // millivolts low, so the limit is moved down out of the way.
    pmic.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V04);
    pmic.enableVbusVoltageMeasure();
}

void update() {}

uint16_t vbusMillivolts() { return pmicOk ? pmic.getVbusVoltage() : 0; }

const char* name() { return "Waveshare AMOLED 1.75"; }

}  // namespace board
