#include "face_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string.h>

#include <memory>
#include <vector>

#include "crc32.h"
#include "image_store.h"

using namespace hg;

namespace face_store {

namespace {

const char* kFile = "/config.json";
const char* kTempFile = "/config.tmp";
const char* kPrefs = "gauge";
const char* kBootsKey = "boots";
const char* kFaceKey = "face";
// Restarts in a row, without ever running long enough to clear the count,
// after which the stored config is left alone and the built-in face shown.
const uint32_t kBootLoopLimit = 3;

// The face a gauge shows with no config of its own: the classic tach template.
const char kBuiltIn[] =
    R"({"schema":1,"name":"Built-in","theme":"white","faces":[{"name":"Tach","widgets":[)"
    R"({"type":"dial","channel":"rpm","min":0,"max":8000,"major":8,"minor":5,"labelScale":1000,)"
    R"("labelFont":"condensed","labelSize":26,"needle":"tapered_cap","zones":[{"from":6800,"to":8000,"color":"alert"}]},)"
    R"({"type":"number","channel":"rpm","y":320,"font":"digital","size":56},)"
    R"({"type":"label","text":"RPM x1000","y":150,"size":22,"color":"dim"},)"
    R"({"type":"light","channel":"check_engine_light","y":400,"r":10,"text":"CEL"}]}]})";

SemaphoreHandle_t mtx = nullptr;

// Shared between the tasks, under mtx.
std::unique_ptr<cfg::Config> staged;    // parsed, waiting for the UI to take it
bool stagedIsTry = false;               // shown without storing
bool stagedIsTrial = false;             // and a preview from the page, noted on the screen
bool stagedValid = false;

// UI task only.
std::unique_ptr<cfg::Config> active;
std::unique_ptr<cfg::Config> stored;    // kept while a try config is shown, to go back to
bool activeIsTry = false;
bool activeIsTrial = false;
uint8_t face = 0;
bool inFallback = false;

// CAN task only.
std::vector<uint8_t> storedData;
uint32_t crc = 0;
uint8_t* rxBuffer = nullptr;

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

std::unique_ptr<cfg::Config> parse(const uint8_t* data, size_t size, const char* what) {
    std::unique_ptr<cfg::Config> c(new cfg::Config);
    const cfg::ParseResult r = cfg::parseConfig(data, size, *c);
    if (!r.ok) {
        Serial.printf("%s config refused: %s\n", what, r.error.c_str());
        return nullptr;
    }
    return c;
}

std::unique_ptr<cfg::Config> builtIn() {
    return parse(reinterpret_cast<const uint8_t*>(kBuiltIn), sizeof(kBuiltIn) - 1, "built-in");
}

// Once a config is in use the images it does not name can go.
void dropUnusedImages(const cfg::Config& c) {
    uint32_t crcs[cfg::kMaxImagesPerConfig];
    size_t n = 0;
    for (const cfg::ImageRef& ref : c.images) {
        if (n < cfg::kMaxImagesPerConfig) crcs[n++] = ref.crc;
    }
    image_store::keepOnly(crcs, n);
}

void stage(std::unique_ptr<cfg::Config> c, bool isTry, bool isTrial = false) {
    Guard guard;
    staged = std::move(c);
    stagedIsTry = isTry;
    stagedIsTrial = isTry && isTrial;
    stagedValid = true;
}

void freeRxBuffer() {
    free(rxBuffer);
    rxBuffer = nullptr;
}

uint32_t readBoots() {
    Preferences p;
    p.begin(kPrefs, true);
    const uint32_t boots = p.getUInt(kBootsKey, 0);
    p.end();
    return boots;
}

void writeBoots(uint32_t boots) {
    Preferences p;
    p.begin(kPrefs, false);
    p.putUInt(kBootsKey, boots);
    p.end();
}

}  // namespace

void begin() {
    mtx = xSemaphoreCreateMutex();

    Preferences p;
    p.begin(kPrefs, true);
    face = uint8_t(p.getUInt(kFaceKey, 0));
    p.end();

    // A config that crashes the renderer would restart the gauge for ever;
    // after a few restarts without a healthy run it is left alone.
    const uint32_t boots = readBoots() + 1;
    writeBoots(boots);
    inFallback = boots > kBootLoopLimit;

    File f = LittleFS.open(kFile, "r");
    if (f) {
        storedData.resize(f.size());
        const size_t got = storedData.empty() ? 0 : f.read(storedData.data(), storedData.size());
        f.close();
        if (got != storedData.size()) storedData.clear();
    }
    if (!storedData.empty()) {
        crc = crc32(storedData.data(), storedData.size());
        if (inFallback) {
            Serial.printf("Restarted %lu times without settling: showing the built-in face instead of the stored config\n",
                          (unsigned long)boots);
        } else {
            stored = parse(storedData.data(), storedData.size(), "stored");
        }
    }
    if (!stored) stored = builtIn();
    if (stored) dropUnusedImages(*stored);
    active.reset(stored ? new cfg::Config(*stored) : nullptr);
    if (!active) active = builtIn();  // the built-in parses; this is belt and braces
    if (face >= active->faces.size()) face = 0;
}

proto::XferStatus accept(proto::XferKind kind, uint32_t size, uint8_t** buffer) {
    if (size > cfg::kMaxConfigBytes + cfg::kMaxImageMapBytes) return proto::XferStatus::TooBig;
    (void)kind;
    freeRxBuffer();
    const size_t bytes = size ? size : 1;
    rxBuffer = static_cast<uint8_t*>(heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!rxBuffer) rxBuffer = static_cast<uint8_t*>(malloc(bytes));
    if (!rxBuffer) return proto::XferStatus::Busy;
    *buffer = rxBuffer;
    return proto::XferStatus::Ok;
}

bool complete(proto::XferKind kind, const uint8_t* data, uint32_t size) {
    const bool trial = kind == proto::XferKind::TryConfig;
    const bool ramOnly = trial || kind == proto::XferKind::SwitchConfig;
    std::unique_ptr<cfg::Config> c = parse(data, size, trial ? "try" : ramOnly ? "switched-to" : "pushed");
    if (!c) {
        freeRxBuffer();
        return false;
    }
    if (ramOnly) {
        stage(std::move(c), true, trial);
        freeRxBuffer();
        return true;
    }

    // Stored as one write and a rename, so a power cut leaves the old file whole.
    File f = LittleFS.open(kTempFile, "w");
    if (!f || f.write(data, size) != size) {
        if (f) f.close();
        LittleFS.remove(kTempFile);
        freeRxBuffer();
        Serial.println("pushed config not stored: no room");
        return false;
    }
    f.close();
    LittleFS.remove(kFile);
    if (!LittleFS.rename(kTempFile, kFile)) {
        freeRxBuffer();
        return false;
    }
    storedData.assign(data, data + size);
    crc = crc32(data, size);
    // A fresh config is a fresh start for the restart count.
    writeBoots(0);
    inFallback = false;
    dropUnusedImages(*c);
    const size_t faces = c->faces.size();
    stage(std::move(c), false);
    freeRxBuffer();
    Serial.printf("Config stored: %u bytes, %u face(s), CRC %08lX\n", unsigned(size), unsigned(faces),
                  (unsigned long)crc);
    return true;
}

void dropped() { freeRxBuffer(); }

const cfg::Config* current() { return active.get(); }

bool takeChange() {
    std::unique_ptr<cfg::Config> next;
    bool isTry = false;
    bool isTrial = false;
    {
        Guard guard;
        if (!stagedValid) return false;
        next = std::move(staged);
        isTry = stagedIsTry;
        isTrial = stagedIsTrial;
        stagedValid = false;
    }
    if (!next) return false;
    if (isTry) {
        if (!activeIsTry) stored = std::move(active);  // keep what to go back to
        activeIsTry = true;
        activeIsTrial = isTrial;
    } else {
        stored.reset(new cfg::Config(*next));
        activeIsTry = false;
        activeIsTrial = false;
    }
    active = std::move(next);
    if (face >= active->faces.size()) face = 0;
    return true;
}

size_t faceCount() { return active ? active->faces.size() : 0; }

uint8_t activeFace() { return face; }

void setActiveFace(uint8_t index) {
    const size_t count = faceCount();
    if (count == 0) return;
    const uint8_t next = uint8_t(index < count ? index : count - 1);
    if (next == face) return;
    face = next;
    Preferences p;
    p.begin(kPrefs, false);
    p.putUInt(kFaceKey, face);
    p.end();
}

bool trying() { return activeIsTry; }

bool trial() { return activeIsTry && activeIsTrial; }

void endTry() {
    if (!activeIsTry) return;
    // Staged so the UI task swaps it in on its own schedule.
    stage(std::unique_ptr<cfg::Config>(new cfg::Config(*stored)), false);
}

void markHealthy() {
    static bool done = false;
    if (done) return;
    done = true;
    if (readBoots() != 0) writeBoots(0);
}

bool fallback() { return inFallback; }

uint32_t storedCrc() { return crc; }

const uint8_t* storedBytes(size_t& size) {
    size = storedData.size();
    return storedData.empty() ? nullptr : storedData.data();
}

}  // namespace face_store
