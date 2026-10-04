#include "recorder.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "haltech.h"

using namespace hg;

namespace recorder {

namespace {

const char* kDir = "/rec";
const size_t kLineCap = 6144;          // the first row lists every live channel
const uint32_t kBytesPerSecondGuess = 4000;

bool running = false;
File file;
std::string current;
uint32_t startMs = 0;
uint32_t endMs = 0;
uint32_t lastRowMs = 0;
uint32_t rows = 0;
uint32_t written = 0;  // the filesystem only reports the size on close
float last[CH_COUNT];
bool had[CH_COUNT];
char* line = nullptr;

std::string pathFor(const char* name) { return std::string(kDir) + "/" + name + ".json"; }

void writeHeader(uint32_t nowMs) {
    file.print("{\"kind\":\"haltech-gauges-recording\",\"schema\":1,\"name\":\"");
    file.print(current.c_str());
    file.printf("\",\"rate_hz\":%u,\"recorded_uptime_s\":%lu,\"channels\":[", unsigned(kRateHz),
                (unsigned long)(nowMs / 1000));
    for (int i = 0; i < CH_COUNT; i++) {
        file.printf("%s\"%s\"", i ? "," : "", channelDef(ChannelId(i)).name);
    }
    file.print("],\"rows\":[\n");
}

}  // namespace

void begin() {
    if (!LittleFS.exists(kDir)) LittleFS.mkdir(kDir);
    LittleFS.remove("/rec/.upload.tmp");
}

bool validName(const char* name) {
    if (!name) return false;
    const size_t len = strlen(name);
    if (len == 0 || len > 32) return false;
    for (size_t i = 0; i < len; i++) {
        const char c = name[i];
        if (!isalnum(uint8_t(c)) && c != '-' && c != '_') return false;
    }
    return true;
}

bool exists(const char* name) { return validName(name) && LittleFS.exists(pathFor(name).c_str()); }

std::string filePath(const char* name) { return pathFor(name); }

bool start(const char* name, uint32_t seconds, std::string& error) {
    if (running) {
        error = "already recording";
        return false;
    }
    if (!validName(name)) {
        error = "name must be 1-32 characters: letters, digits, - and _";
        return false;
    }
    if (seconds < 1 || seconds > kMaxSeconds) {
        error = "length must be 1 to 180 seconds";
        return false;
    }
    const size_t need = size_t(seconds) * kBytesPerSecondGuess + 32768;
    if (LittleFS.totalBytes() - LittleFS.usedBytes() < need) {
        error = "not enough storage on the hub; delete a recording or a config";
        return false;
    }
    line = static_cast<char*>(malloc(kLineCap));
    if (!line) {
        error = "hub is short of memory";
        return false;
    }
    file = LittleFS.open(pathFor(name).c_str(), "w");
    if (!file) {
        free(line);
        line = nullptr;
        error = "cannot create the file";
        return false;
    }
    current = name;
    const uint32_t now = millis();
    startMs = lastRowMs = now;
    endMs = now + seconds * 1000;
    rows = 0;
    written = 0;
    memset(had, 0, sizeof(had));
    writeHeader(now);
    written = file.position();
    running = true;
    return true;
}

void stop() {
    if (!running) return;
    running = false;
    file.printf("[%lu,[]]\n],\"duration_ms\":%lu}\n", (unsigned long)(millis() - startMs),
                (unsigned long)(millis() - startMs));
    file.close();
    free(line);
    line = nullptr;
}

bool active() { return running; }

void poll(uint32_t nowMs, const ChannelStore& store) {
    if (!running) return;
    if (int32_t(nowMs - endMs) >= 0) {
        stop();
        return;
    }
    if (nowMs - lastRowMs < 1000 / kRateHz) return;
    lastRowMs += 1000 / kRateHz;
    if (nowMs - lastRowMs > 500) lastRowMs = nowMs;  // fell behind: resync rather than burst

    size_t n = snprintf(line, kLineCap, "[%lu,[", (unsigned long)(nowMs - startMs));
    bool any = false;
    for (int i = 0; i < CH_COUNT; i++) {
        float v;
        if (!store.get(ChannelId(i), nowMs, v)) continue;
        if (had[i] && v == last[i]) continue;
        if (n + 40 >= kLineCap) break;  // row full; the rest goes in the next one
        had[i] = true;
        last[i] = v;
        n += snprintf(line + n, kLineCap - n, "%s%d,%.6g", any ? "," : "", i, double(v));
        any = true;
    }
    if (!any && rows > 0) return;  // nothing moved: no row
    n += snprintf(line + n, kLineCap - n, "]],\n");
    if (file.write(reinterpret_cast<const uint8_t*>(line), n) != n) {
        stop();  // storage full: keep what we have
        return;
    }
    rows++;
    written += n;
}

void statusJson(JsonObject out, uint32_t nowMs) {
    out["active"] = running;
    if (!running) return;
    out["name"] = current;
    out["elapsed_ms"] = nowMs - startMs;
    out["seconds"] = (endMs - startMs) / 1000;
    out["rows"] = rows;
    out["bytes"] = written;
}

uint32_t durationOf(const char* path) {
    File f = LittleFS.open(path, "r");
    if (!f) return 0;
    const size_t size = f.size();
    char tail[64] = {};
    if (size > sizeof(tail) - 1) f.seek(size - (sizeof(tail) - 1));
    const size_t got = f.read(reinterpret_cast<uint8_t*>(tail), sizeof(tail) - 1);
    f.close();
    tail[got] = 0;
    const char* p = strstr(tail, "\"duration_ms\":");
    return p ? uint32_t(strtoul(p + 14, nullptr, 10)) : 0;
}

void listJson(JsonArray out) {
    File dir = LittleFS.open(kDir);
    if (!dir) return;
    for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
        if (f.isDirectory()) continue;
        String base = f.name();
        const int slash = base.lastIndexOf('/');
        if (slash >= 0) base = base.substring(slash + 1);
        if (base.startsWith(".") || !base.endsWith(".json")) continue;
        base.remove(base.length() - 5);
        JsonObject o = out.add<JsonObject>();
        o["name"] = base;
        o["bytes"] = f.size();
        const bool inProgress = running && current == base.c_str();
        o["duration_ms"] = inProgress ? 0 : durationOf(pathFor(base.c_str()).c_str());
        o["recording"] = inProgress;
    }
}

bool remove(const char* name) {
    if (!exists(name)) return false;
    if (running && current == name) return false;
    return LittleFS.remove(pathFor(name).c_str());
}

bool accept(const char* tempPath, const char* name, std::string& error) {
    if (!validName(name)) {
        error = "name must be 1-32 characters: letters, digits, - and _";
        LittleFS.remove(tempPath);
        return false;
    }
    File f = LittleFS.open(tempPath, "r");
    if (!f) {
        error = "nothing was uploaded";
        return false;
    }
    char head[48] = {};
    const size_t got = f.read(reinterpret_cast<uint8_t*>(head), sizeof(head) - 1);
    f.close();
    head[got] = 0;
    if (strncmp(head, "{\"kind\":\"haltech-gauges-recording\"", 34) != 0) {
        LittleFS.remove(tempPath);
        error = "that file is not a recording";
        return false;
    }
    if (running && current == name) {
        LittleFS.remove(tempPath);
        error = "that name is being recorded right now";
        return false;
    }
    const std::string path = pathFor(name);
    LittleFS.remove(path.c_str());
    if (!LittleFS.rename(tempPath, path.c_str())) {
        error = "could not store the recording";
        return false;
    }
    return true;
}

}  // namespace recorder
