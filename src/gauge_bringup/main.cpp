// Gauge board bring-up (step 1). No LVGL, no CAN: just proves the display,
// touch, PMIC and rear header on a bare Waveshare ESP32-S3-Touch-AMOLED-1.75.
//
// Press the BOOT button to step through the stages:
//   1 colour bars     2 brightness ramp     3 full white (measure current)
//   4 touch test      5 header pin mapper
// Everything is also reported on the USB serial port at 115200.

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>

#define XPOWERS_CHIP_AXP2101
#include <TouchDrvCSTXXX.hpp>
#include <XPowersLib.h>

#include "../gauge/board_pins.h"

namespace {

const int kBootButton = 0;
const int kScreen = 466;
const uint16_t kBlack = 0x0000, kWhite = 0xFFFF, kRed = 0xF800, kGreen = 0x07E0, kBlue = 0x001F,
               kYellow = 0xFFE0, kGrey = 0x4208;

Arduino_DataBus* bus = new Arduino_ESP32QSPI(PIN_LCD_CS, PIN_LCD_SCLK, PIN_LCD_D0, PIN_LCD_D1,
                                             PIN_LCD_D2, PIN_LCD_D3);
// The panel's visible area starts 6 columns into the controller's memory.
Arduino_CO5300* gfx = new Arduino_CO5300(bus, PIN_LCD_RST, 0, kScreen, kScreen, 6, 0, 0, 0);

TouchDrvCST92xx touch;
XPowersPMU pmic;
bool touchOk = false;
bool pmicOk = false;

enum Stage { ColourBars, BrightnessRamp, FullWhite, TouchTest, PinMapper, StageCount };
int stage = ColourBars;
uint32_t stageStartMs = 0;

const int kHeaderPins[] = {16, 17, 18};
int highPin = -1;

void title(const char* text, uint16_t fg = kWhite, uint16_t bg = kBlack) {
    gfx->fillRect(80, 40, kScreen - 160, 36, bg);
    gfx->setTextSize(2);
    gfx->setTextColor(fg);
    gfx->setCursor(100, 50);
    gfx->print(text);
    Serial.printf("--- %s ---\n", text);
}

void scanI2c() {
    Serial.print("I2C devices:");
    for (uint8_t addr = 8; addr < 120; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) Serial.printf(" 0x%02X", addr);
    }
    Serial.println();
    Serial.println("  expected: 0x18 codec, 0x20 expander, 0x34 PMIC, 0x40 mic ADC, 0x51 RTC, "
                   "0x5A touch, 0x6B IMU");
}

void reportPower() {
    if (!pmicOk) return;
    Serial.printf("VBUS %u mV, system %u mV\n", pmic.getVbusVoltage(), pmic.getSystemVoltage());
}

void enterStage(int next) {
    stage = next % StageCount;
    stageStartMs = millis();
    gfx->setBrightness(200);
    for (int pin : kHeaderPins) digitalWrite(pin, LOW);
    highPin = -1;

    switch (stage) {
        case ColourBars: {
            const uint16_t colours[] = {kRed, kGreen, kBlue, kWhite, kYellow};
            const int bandWidth = kScreen / 5;
            for (int i = 0; i < 5; i++) {
                gfx->fillRect(i * bandWidth, 0, bandWidth + 1, kScreen, colours[i]);
            }
            title("1 Colour bars", kBlack, kWhite);
            break;
        }
        case BrightnessRamp:
            gfx->fillScreen(kWhite);
            title("2 Brightness ramp", kBlack, kWhite);
            break;
        case FullWhite:
            gfx->fillScreen(kWhite);
            gfx->setBrightness(255);
            title("3 Full white", kBlack, kWhite);
            Serial.println("Measure the supply current now: this is the worst case.");
            break;
        case TouchTest:
            gfx->fillScreen(kBlack);
            title("4 Touch: draw");
            gfx->drawCircle(kScreen / 2, kScreen / 2, 100, kGrey);
            gfx->drawFastHLine(0, kScreen / 2, kScreen, kGrey);
            gfx->drawFastVLine(kScreen / 2, 0, kScreen, kGrey);
            if (!touchOk) Serial.println("Touch controller did not start.");
            break;
        case PinMapper:
            gfx->fillScreen(kBlack);
            title("5 Header pins");
            Serial.println("One of GPIO16/17/18 is driven to 3.3 V at a time, the other two to 0 V.");
            Serial.println("Probe header positions 6, 7 and 8 and note which GPIO each one is.");
            break;
    }
}

void runStage() {
    const uint32_t elapsed = millis() - stageStartMs;
    switch (stage) {
        case BrightnessRamp: {
            // Triangle wave 0..255..0 over 5 seconds.
            const uint32_t phase = elapsed % 5000;
            const uint32_t level = phase < 2500 ? phase * 255 / 2500 : (5000 - phase) * 255 / 2500;
            gfx->setBrightness(uint8_t(level));
            delay(20);
            break;
        }
        case TouchTest: {
            if (!touchOk) break;
            int16_t x[2], y[2];
            const uint8_t points = touch.getPoint(x, y, 2);
            for (uint8_t i = 0; i < points; i++) {
                gfx->fillCircle(x[i], y[i], 6, i == 0 ? kGreen : kYellow);
                Serial.printf("touch %u: x %d  y %d\n", i, x[i], y[i]);
            }
            delay(10);
            break;
        }
        case PinMapper: {
            const int index = int((elapsed / 4000) % 3);
            if (kHeaderPins[index] != highPin) {
                highPin = kHeaderPins[index];
                for (int pin : kHeaderPins) digitalWrite(pin, pin == highPin ? HIGH : LOW);
                gfx->fillRect(60, 190, kScreen - 120, 90, kBlack);
                gfx->setTextSize(5);
                gfx->setTextColor(kGreen);
                gfx->setCursor(110, 200);
                gfx->printf("GPIO%d", highPin);
                gfx->setTextSize(2);
                gfx->setTextColor(kWhite);
                gfx->setCursor(150, 255);
                gfx->print("is at 3.3 V");
                Serial.printf("GPIO%d high\n", highPin);
            }
            break;
        }
        default:
            break;
    }
}

}  // namespace

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);
    delay(1500);
    Serial.println("\nGauge board bring-up");
    Serial.printf("Chip %s rev %d, flash %u MB, PSRAM %u bytes (expect about 8 MB)\n",
                  ESP.getChipModel(), ESP.getChipRevision(),
                  unsigned(ESP.getFlashChipSize() / (1024 * 1024)), unsigned(ESP.getPsramSize()));

    pinMode(kBootButton, INPUT_PULLUP);
    for (int pin : kHeaderPins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    scanI2c();

    pmicOk = pmic.begin(Wire, AXP2101_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL);
    Serial.printf("PMIC %s\n", pmicOk ? "ok" : "NOT FOUND");
    if (pmicOk) {
        pmic.enableVbusVoltageMeasure();
        pmic.enableSystemVoltageMeasure();
        Serial.printf("Powered on by %s\n", pmic.isVbusInsertOnSource() ? "5 V being applied" : "another source");
        reportPower();
    }

    if (!gfx->begin()) Serial.println("Display did not start");
    gfx->fillScreen(kBlack);

    touch.setPins(PIN_TOUCH_RST, PIN_TOUCH_INT);
    touchOk = touch.begin(Wire, 0x5A, PIN_I2C_SDA, PIN_I2C_SCL);
    Serial.printf("Touch %s\n", touchOk ? touch.getModelName() : "NOT FOUND");
    if (touchOk) {
        touch.setMaxCoordinates(kScreen, kScreen);
        touch.setMirrorXY(true, true);
    }

    enterStage(ColourBars);
}

void loop() {
    static bool wasDown = false;
    const bool down = digitalRead(kBootButton) == LOW;
    if (down && !wasDown) {
        delay(30);  // debounce
        enterStage(stage + 1);
    }
    wasDown = down;

    runStage();

    static uint32_t lastReport = 0;
    if (millis() - lastReport >= 2000) {
        lastReport = millis();
        reportPower();
    }
}
