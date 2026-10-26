#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

// The wheel's buttons, read as voltages on resistor-ladder lines
// (docs/steering-wheel.md). Each line has a pull-up; a button pulls it to a
// voltage of its own. A calibration table names the voltage of each button
// on each line; it is kept in the preferences and the hub holds a copy.
namespace buttons {

constexpr size_t kMaxLines = 4;
constexpr size_t kMaxButtonsPerLine = 8;
constexpr uint16_t kDefaultToleranceMv = 80;

// The button names the calibration uses, in kInput... bit order.
constexpr size_t kNames = 14;
extern const char* const kButtonNames[kNames];

struct Button {
    char name[16] = "";
    uint8_t bit = 0;    // the kInput... bit
    uint16_t mv = 0;    // the line's reading with this button down
    uint16_t tol = kDefaultToleranceMv;
};
struct Line {
    uint16_t pullupOhms = 2200;  // documentation; the readings are what count
    uint8_t count = 0;
    Button button[kMaxButtonsPerLine];
};
struct Calibration {
    uint8_t lineCount = 0;
    Line line[kMaxLines];
};

void begin();
// Call every millisecond: samples the lines and debounces the buttons.
void sample(uint32_t nowMs);

// The buttons down right now, as kInput... bits. Always 0 until calibrated.
uint16_t mask();
// A line reads where no button and no idle sits, for longer than a moment:
// a short or a broken wire.
bool fault();
bool calibrated();
size_t lineCount();  // lines being read (3 by default, 4 with PIN_LINE_3 enabled)
uint16_t rawMv(size_t line);

// The calibration as JSON, and from JSON (stored in the preferences);
// its CRC32 is what the Status message reports.
bool setCalibration(const char* json, size_t len, std::string& error);
std::string calibrationJson();
uint32_t calibrationCrc();
void clearCalibration();

// From the console: record the current reading of `line` as `name`'s centre
// (in RAM; `save()` keeps it), set the tolerance, write to the preferences.
bool capture(size_t line, const char* name, std::string& error);
void setTolerance(uint16_t mv);
bool save();

}  // namespace buttons
