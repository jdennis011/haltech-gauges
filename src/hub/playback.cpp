#include "playback.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <stdlib.h>
#include <string.h>

#include "hub_state.h"
#include "recorder.h"
#include "simulator.h"

using namespace hg;

namespace playback {

namespace {

const size_t kMaxRecChannels = 512;

bool running = false;
bool holding = false;  // finished, last values kept until stop()
bool looping = false;
File file;
std::string current;
size_t firstRowPos = 0;
int16_t map_[kMaxRecChannels];  // index in the recording -> ChannelId, or -1
size_t mapCount = 0;
float vals[CH_COUNT];
bool has[CH_COUNT];
double position = 0;  // ms into the recording
uint32_t lastReal = 0;
uint32_t durationMs = 0;
bool ended = false;

bool haveNext = false;
uint32_t nextT = 0;
std::string nextLine;

bool readLine(std::string& out) {
    out.clear();
    while (file.available()) {
        const int c = file.read();
        if (c < 0) break;
        if (c == '\n') return true;
        out.push_back(char(c));
        if (out.size() > 16384) return false;
    }
    return !out.empty();
}

// "[t,[i,v,i,v,...]]," : the time, and the values when apply is set.
bool parseRow(const std::string& l, uint32_t& t, bool apply) {
    const char* p = l.c_str();
    if (*p != '[') return false;
    char* e;
    t = uint32_t(strtoul(p + 1, &e, 10));
    p = e;
    if (*p != ',' || p[1] != '[') return false;
    p += 2;
    while (*p && *p != ']') {
        const long idx = strtol(p, &e, 10);
        if (e == p || *e != ',') return false;
        p = e + 1;
        const float v = strtof(p, &e);
        if (e == p) return false;
        p = e;
        if (apply && idx >= 0 && size_t(idx) < mapCount && map_[idx] >= 0) {
            vals[map_[idx]] = v;
            has[map_[idx]] = true;
        }
        if (*p == ',') p++;
    }
    return true;
}

bool parseHeader(const std::string& h) {
    JsonDocument filter;
    filter["channels"] = true;
    JsonDocument doc;
    std::string s = h;
    s += "]}";  // the header line ends with "rows":[ ; close it to make it a document
    if (deserializeJson(doc, s, DeserializationOption::Filter(filter))) return false;
    mapCount = 0;
    for (JsonVariantConst v : doc["channels"].as<JsonArrayConst>()) {
        if (mapCount >= kMaxRecChannels) break;
        const char* name = v.as<const char*>();
        map_[mapCount++] = int16_t(name ? findChannel(name) : -1);
    }
    return mapCount > 0;
}

// Reads the next row's line and time, outside the lock.
void advance() {
    haveNext = false;
    if (readLine(nextLine) && parseRow(nextLine, nextT, false)) {
        haveNext = true;
        return;
    }
    // Trailer or end of file.
    if (looping && file.seek(firstRowPos)) {
        position -= durationMs ? durationMs : position;
        if (readLine(nextLine) && parseRow(nextLine, nextT, false)) haveNext = true;
        return;
    }
    ended = true;
}

}  // namespace

bool start(const char* name, bool loop, std::string& error) {
    stop();
    if (!recorder::exists(name)) {
        error = "no such recording";
        return false;
    }
    const std::string path = recorder::filePath(name);
    durationMs = recorder::durationOf(path.c_str());
    file = LittleFS.open(path.c_str(), "r");
    if (!file) {
        error = "cannot open the recording";
        return false;
    }
    std::string header;
    if (!readLine(header) || !parseHeader(header)) {
        file.close();
        error = "the recording has no channel list";
        return false;
    }
    firstRowPos = file.position();
    {
        hub::Lock lock;
        memset(has, 0, sizeof(has));
        current = name;
        looping = loop;
        position = 0;
        lastReal = millis();
        ended = false;
        holding = false;
        running = true;
    }
    advance();
    hub::setSimulator(true);  // the simulator carries the values to the bus and the page
    return true;
}

void stop() {
    hub::Lock lock;
    running = false;
    holding = false;
    haveNext = false;
    memset(has, 0, sizeof(has));
    if (file) file.close();
}

bool active() { return running; }

bool value(ChannelId id, float& out) {
    if (!(running || holding) || !has[id]) return false;
    out = vals[id];
    return true;
}

void poll(uint32_t nowMs) {
    if (!running) return;
    const float rate = simulator::paused() ? 0.0f : simulator::speed();
    position += double(nowMs - lastReal) * rate;
    lastReal = nowMs;
    int applied = 0;
    while (haveNext && double(nextT) <= position && applied < 20) {
        {
            hub::Lock lock;
            uint32_t t;
            parseRow(nextLine, t, true);
        }
        advance();
        applied++;
    }
    if (ended) {
        hub::Lock lock;
        running = false;
        holding = true;
        file.close();
    }
}

void statusJson(JsonObject out) {
    out["active"] = running;
    if (!running && !holding) return;
    out["name"] = current;
    out["position_ms"] = uint32_t(running ? position : durationMs);
    out["duration_ms"] = durationMs;
    out["loop"] = looping;
    out["ended"] = holding;
}

const char* currentName() { return (running || holding) ? current.c_str() : ""; }

}  // namespace playback
