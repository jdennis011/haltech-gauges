#include "buttons.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <string.h>

#include <algorithm>

#include "board_pins.h"
#include "crc32.h"

namespace buttons {

const char* const kButtonNames[kNames] = {"horn",       "cruise_on_off", "cruise_set", "cruise_resume", "cruise_accel",
                                          "cruise_decel", "cruise_cancel", "vol_up",   "vol_down",      "mode",
                                          "mute",       "seek_up",       "seek_down",  "phone"};

namespace {

const char* kPrefs = "wheel";
const char* kCalKey = "cal";
const uint32_t kDebounceMs = 20;
const uint32_t kFaultMs = 500;
const uint16_t kIdleMv = 2300;  // above every button: the pull-up alone (the ADC tops out near 2.5 V)

#ifdef HG_WHEEL_FOUR_LINES
const int kPins[] = {PIN_LINE_0, PIN_LINE_1, PIN_LINE_2, PIN_LINE_3};
#else
const int kPins[] = {PIN_LINE_0, PIN_LINE_1, PIN_LINE_2};
#endif
constexpr size_t kLines = sizeof(kPins) / sizeof(kPins[0]);

Calibration cal;
bool haveCal = false;
uint32_t calCrc = 0;
std::string calText;

struct LineState {
    uint16_t samples[5] = {};
    uint8_t at = 0;
    uint16_t mv = 0;          // the median of the last five
    int8_t candidate = -1;    // button index being debounced, -1 idle, -2 unknown
    uint32_t candidateMs = 0;
    int8_t down = -1;         // the button held, after debounce
    uint32_t unknownSinceMs = 0;
    bool fault = false;
};
LineState lines[kLines];
uint16_t maskNow = 0;
bool faultNow = false;

int nameBit(const char* name) {
    for (size_t i = 0; i < kNames; i++) {
        if (strcmp(name, kButtonNames[i]) == 0) return int(i);
    }
    return -1;
}

// Which button a reading matches on a line: index, -1 idle, -2 none.
int match(const Line& line, uint16_t mv) {
    int best = -2;
    uint16_t bestDist = 0xFFFF;
    for (uint8_t i = 0; i < line.count; i++) {
        const Button& b = line.button[i];
        const uint16_t dist = mv > b.mv ? mv - b.mv : b.mv - mv;
        if (dist <= b.tol && dist < bestDist) {
            best = int(i);
            bestDist = dist;
        }
    }
    if (best >= 0) return best;
    return mv >= kIdleMv ? -1 : -2;
}

uint16_t median5(const uint16_t* s) {
    uint16_t v[5];
    memcpy(v, s, sizeof(v));
    std::sort(v, v + 5);
    return v[2];
}

void rebuildText() {
    JsonDocument doc;
    JsonArray ls = doc["lines"].to<JsonArray>();
    for (uint8_t l = 0; l < cal.lineCount; l++) {
        JsonObject lo = ls.add<JsonObject>();
        lo["pullup_ohms"] = cal.line[l].pullupOhms;
        JsonArray bs = lo["buttons"].to<JsonArray>();
        for (uint8_t i = 0; i < cal.line[l].count; i++) {
            const Button& b = cal.line[l].button[i];
            JsonObject bo = bs.add<JsonObject>();
            bo["name"] = b.name;
            bo["bit"] = b.bit;
            bo["mv"] = b.mv;
            bo["tol"] = b.tol;
        }
    }
    calText.clear();
    serializeJson(doc, calText);
    calCrc = hg::crc32(reinterpret_cast<const uint8_t*>(calText.data()), calText.size());
    haveCal = false;
    for (uint8_t l = 0; l < cal.lineCount; l++) haveCal = haveCal || cal.line[l].count > 0;
}

bool parse(const char* json, size_t len, Calibration& out, std::string& error) {
    JsonDocument doc;
    if (deserializeJson(doc, json, len) != DeserializationError::Ok || !doc["lines"].is<JsonArrayConst>()) {
        error = "the calibration is a JSON object with a 'lines' list";
        return false;
    }
    Calibration c;
    for (JsonObjectConst lo : doc["lines"].as<JsonArrayConst>()) {
        if (c.lineCount >= kMaxLines) {
            error = "at most four lines";
            return false;
        }
        Line& line = c.line[c.lineCount++];
        line.pullupOhms = uint16_t(lo["pullup_ohms"] | 2200);
        for (JsonObjectConst bo : lo["buttons"].as<JsonArrayConst>()) {
            if (line.count >= kMaxButtonsPerLine) {
                error = "at most eight buttons on a line";
                return false;
            }
            Button& b = line.button[line.count];
            const char* name = bo["name"] | "";
            snprintf(b.name, sizeof(b.name), "%s", name);
            int bit = bo["bit"] | -1;
            if (bit < 0) bit = nameBit(b.name);
            if (bit < 0 || bit > 15) {
                error = std::string("button '") + b.name + "' needs a known name or a 'bit' 0 to 15";
                return false;
            }
            b.bit = uint8_t(bit);
            b.mv = uint16_t(bo["mv"] | 0);
            b.tol = uint16_t(bo["tol"] | kDefaultToleranceMv);
            if (b.mv >= kIdleMv) {
                error = std::string("button '") + b.name + "' reads like an idle line";
                return false;
            }
            line.count++;
        }
    }
    out = c;
    return true;
}

void load() {
    Preferences p;
    p.begin(kPrefs, true);
    const String text = p.getString(kCalKey, "");
    p.end();
    std::string error;
    if (text.length() && parse(text.c_str(), text.length(), cal, error)) {
        rebuildText();
    } else {
        cal = Calibration();
        rebuildText();
    }
}

}  // namespace

void begin() {
    analogReadResolution(12);
    for (size_t i = 0; i < kLines; i++) {
        pinMode(kPins[i], INPUT);
        analogSetPinAttenuation(kPins[i], ADC_11db);
    }
    load();
}

void sample(uint32_t nowMs) {
    uint16_t m = 0;
    bool anyFault = false;
    for (size_t i = 0; i < kLines; i++) {
        LineState& s = lines[i];
        const uint32_t mv = analogReadMilliVolts(kPins[i]);
        s.samples[s.at] = uint16_t(mv > 65535 ? 65535 : mv);
        s.at = uint8_t((s.at + 1) % 5);
        s.mv = median5(s.samples);
        const Line* line = i < cal.lineCount ? &cal.line[i] : nullptr;
        const int now = line ? match(*line, s.mv) : (s.mv >= kIdleMv ? -1 : -2);
        if (now != s.candidate) {
            s.candidate = int8_t(now);
            s.candidateMs = nowMs;
        } else if (now != s.down && uint32_t(nowMs - s.candidateMs) >= kDebounceMs) {
            s.down = int8_t(now);
        }
        if (s.down == -2) {
            if (!s.unknownSinceMs) s.unknownSinceMs = nowMs ? nowMs : 1;
            s.fault = uint32_t(nowMs - s.unknownSinceMs) >= kFaultMs;
        } else {
            s.unknownSinceMs = 0;
            s.fault = false;
        }
        if (line && s.down >= 0 && !s.fault) m |= uint16_t(1u << line->button[s.down].bit);
        anyFault = anyFault || s.fault;
    }
    maskNow = haveCal ? m : 0;
    faultNow = anyFault;
}

uint16_t mask() { return maskNow; }
bool fault() { return faultNow; }
bool calibrated() { return haveCal; }
size_t lineCount() { return kLines; }
uint16_t rawMv(size_t line) { return line < kLines ? lines[line].mv : 0; }

bool setCalibration(const char* json, size_t len, std::string& error) {
    Calibration c;
    if (!parse(json, len, c, error)) return false;
    cal = c;
    // Kept byte for byte as the hub sent it, so the CRC the status reports
    // is the CRC of the hub's own copy and the hub can see it landed.
    calText.assign(json, len);
    calCrc = hg::crc32(reinterpret_cast<const uint8_t*>(calText.data()), calText.size());
    haveCal = false;
    for (uint8_t l = 0; l < cal.lineCount; l++) haveCal = haveCal || cal.line[l].count > 0;
    return save();
}

std::string calibrationJson() { return calText; }
uint32_t calibrationCrc() { return calCrc; }

void clearCalibration() {
    cal = Calibration();
    rebuildText();
    Preferences p;
    p.begin(kPrefs, false);
    p.remove(kCalKey);
    p.end();
}

bool capture(size_t lineIndex, const char* name, std::string& error) {
    if (lineIndex >= kLines) {
        error = "no such line";
        return false;
    }
    const int bit = nameBit(name);
    if (bit < 0) {
        error = "unknown button name; the names are listed by '?'";
        return false;
    }
    while (cal.lineCount <= lineIndex) cal.lineCount++;
    Line& line = cal.line[lineIndex];
    const uint16_t mv = lines[lineIndex].mv;
    if (mv >= kIdleMv) {
        error = "the line reads idle: hold the button while capturing";
        return false;
    }
    Button* b = nullptr;
    for (uint8_t i = 0; i < line.count; i++) {
        if (strcmp(line.button[i].name, name) == 0) b = &line.button[i];
    }
    if (!b) {
        if (line.count >= kMaxButtonsPerLine) {
            error = "that line has eight buttons already";
            return false;
        }
        b = &line.button[line.count++];
        snprintf(b->name, sizeof(b->name), "%s", name);
        b->bit = uint8_t(bit);
    }
    b->mv = mv;
    rebuildText();
    return true;
}

void setTolerance(uint16_t mv) {
    for (uint8_t l = 0; l < cal.lineCount; l++) {
        for (uint8_t i = 0; i < cal.line[l].count; i++) cal.line[l].button[i].tol = mv;
    }
    rebuildText();
}

bool save() {
    Preferences p;
    if (!p.begin(kPrefs, false)) return false;
    const bool ok = p.putString(kCalKey, calText.c_str()) == calText.size();
    p.end();
    return ok;
}

}  // namespace buttons
