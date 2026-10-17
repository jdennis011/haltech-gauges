#include "gauge_firmware.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string.h>

#include "firmware_update.h"
#include "hub_config.h"
#include "update_manifest.h"

using hg::update::Manifest;

namespace gauge_firmware {

namespace {

const char* kDir = "/gfw";
const uint32_t kReserveBytes = 128 * 1024;  // left free for configs and images
const uint32_t kMinHeap = 96 * 1024;
const size_t kMaxManifest = 8 * 1024;

// Info.hwRevision values the gauge firmware reports (proto.h).
const Board kBoards[] = {
    {1, "gauge-ws175", "Waveshare 1.75\" AMOLED gauge"},
    {2, "gauge-m5dial", "M5Stack Dial (development gauge)"},
};
const size_t kBoardCount = sizeof(kBoards) / sizeof(kBoards[0]);

enum class State : uint8_t { Idle, Checking, Fetching, Error };

const char* stateName(State s) {
    switch (s) {
        case State::Idle: return "idle";
        case State::Checking: return "checking";
        case State::Fetching: return "fetching";
        case State::Error: return "error";
    }
    return "idle";
}

struct Slot {
    bool haveStored = false;
    Build stored;
    bool haveLatest = false;
    Manifest latest;
};

SemaphoreHandle_t mtx = nullptr;
// Under mtx.
Slot slots[kBoardCount];
State state = State::Idle;
std::string error;
uint32_t progress = 0;
uint32_t checkedMs = 0;
bool everChecked = false;
bool checkWanted = false;
int fetchWanted = -1;  // index into kBoards
bool working = false;

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

std::string binPath(const Board& b) { return std::string(kDir) + "/" + b.key + ".bin"; }
std::string tmpPath(const Board& b) { return std::string(kDir) + "/" + b.key + ".tmp"; }
std::string notePath(const Board& b) { return std::string(kDir) + "/" + b.key + ".json"; }

int indexOf(uint8_t id) {
    for (size_t i = 0; i < kBoardCount; i++) {
        if (kBoards[i].id == id) return int(i);
    }
    return -1;
}

// The note beside a stored build: what it is.
void loadStored(size_t i) {
    Slot& s = slots[i];
    s.haveStored = false;
    File f = LittleFS.open(notePath(kBoards[i]).c_str(), "r");
    if (!f) return;
    JsonDocument doc;
    const bool ok = deserializeJson(doc, f) == DeserializationError::Ok;
    f.close();
    if (!ok || !LittleFS.exists(binPath(kBoards[i]).c_str())) return;
    s.stored.version = doc["version"] | "";
    s.stored.size = doc["size"] | 0;
    s.stored.sha256 = doc["sha256"] | "";
    s.stored.path = binPath(kBoards[i]);
    s.haveStored = !s.stored.version.empty() && s.stored.size > 0;
}

void saveNote(size_t i, const Manifest& m) {
    JsonDocument doc;
    doc["version"] = m.version;
    doc["size"] = m.size;
    doc["sha256"] = m.sha256;
    doc["date"] = m.date;
    File f = LittleFS.open(notePath(kBoards[i]).c_str(), "w");
    if (!f) return;
    serializeJson(doc, f);
    f.close();
}

void setState(State s, const char* err = nullptr) {
    Guard guard;
    state = s;
    if (err) error = err;
}

bool toString(const uint8_t* data, size_t len, void* ctx) {
    std::string* s = static_cast<std::string*>(ctx);
    if (s->size() + len > kMaxManifest) return false;
    s->append(reinterpret_cast<const char*>(data), len);
    return true;
}

std::string siteUrl(const Board& b, const char* file) {
    return std::string(HUB_CLOUD_URL) + "/fw/" + b.key + "/" + file;
}

// Reads the site's manifest for one board into its slot. False, with why, on anything but success or "none".
bool checkBoard(size_t i, std::string& why) {
    std::string body;
    int64_t length;
    const int status = firmware_update::fetch(siteUrl(kBoards[i], "manifest.json").c_str(), toString, &body, why, length);
    if (status < 0) return false;
    Guard guard;
    if (status == 404) {
        slots[i].haveLatest = false;
        return true;
    }
    if (status != 200) {
        char buf[40];
        snprintf(buf, sizeof(buf), "the site answered HTTP %d", status);
        why = buf;
        return false;
    }
    Manifest m;
    if (!hg::update::parseManifest(reinterpret_cast<const uint8_t*>(body.data()), body.size(), kBoards[i].key, m, why)) {
        return false;
    }
    slots[i].latest = m;
    slots[i].haveLatest = true;
    return true;
}

void check() {
    setState(State::Checking, "");
    std::string why;
    bool ok = true;
    for (size_t i = 0; i < kBoardCount && ok; i++) ok = checkBoard(i, why);
    Guard guard;
    checkedMs = millis();
    everChecked = true;
    state = ok ? State::Idle : State::Error;
    if (!ok) error = why;
}

struct Download {
    File file;
    mbedtls_sha256_context sha;
    uint32_t written = 0;
    uint32_t expected = 0;
    std::string why;
};

bool toFile(const uint8_t* data, size_t len, void* ctx) {
    Download* d = static_cast<Download*>(ctx);
    if (d->written + len > d->expected) {
        d->why = "the file is larger than the manifest says";
        return false;
    }
    if (d->file.write(data, len) != len) {
        d->why = "the hub's storage is full";
        return false;
    }
    mbedtls_sha256_update(&d->sha, data, len);
    d->written += len;
    {
        Guard guard;
        progress = d->expected ? uint32_t(uint64_t(d->written) * 100 / d->expected) : 0;
    }
    return true;
}

void fetch(size_t i) {
    Manifest m;
    {
        Guard guard;
        if (!slots[i].haveLatest) {
            state = State::Error;
            error = "the site has no build for this board: check first";
            return;
        }
        m = slots[i].latest;
        working = true;
    }
    const Board& b = kBoards[i];
    std::string why;
    bool ok = false;
    do {
        if (ESP.getFreeHeap() < kMinHeap) {
            why = "not enough free memory; try again in a moment";
            break;
        }
        const uint32_t free = LittleFS.totalBytes() - LittleFS.usedBytes();
        uint32_t reclaim = 0;  // the build being replaced goes
        {
            Guard guard;
            if (slots[i].haveStored) reclaim = slots[i].stored.size;
        }
        if (free + reclaim < m.size + kReserveBytes) {
            char buf[120];
            snprintf(buf, sizeof(buf), "not enough room on the hub's storage: %lu KB free, the build is %lu KB",
                     (unsigned long)((free + reclaim) / 1024), (unsigned long)(m.size / 1024));
            why = buf;
            break;
        }
        if (reclaim) {
            Guard guard;
            slots[i].haveStored = false;
            LittleFS.remove(binPath(b).c_str());
            LittleFS.remove(notePath(b).c_str());
        }
        setState(State::Fetching, "");
        {
            Guard guard;
            progress = 0;
        }
        Download d;
        d.expected = m.size;
        d.file = LittleFS.open(tmpPath(b).c_str(), "w");
        if (!d.file) {
            why = "could not write to the hub's storage";
            break;
        }
        mbedtls_sha256_init(&d.sha);
        mbedtls_sha256_starts(&d.sha, 0);
        int64_t length;
        const int status = firmware_update::fetch(siteUrl(b, "firmware.bin").c_str(), toFile, &d, d.why, length);
        uint8_t digest[32];
        mbedtls_sha256_finish(&d.sha, digest);
        mbedtls_sha256_free(&d.sha);
        d.file.close();
        if (status < 0) {
            why = d.why;
            break;
        }
        if (status != 200) {
            char buf[40];
            snprintf(buf, sizeof(buf), "the site answered HTTP %d", status);
            why = buf;
            break;
        }
        if (d.written != m.size) {
            why = "the download was cut short";
            break;
        }
        char hex[65];
        for (int k = 0; k < 32; k++) snprintf(hex + 2 * k, 3, "%02x", digest[k]);
        if (m.sha256 != hex) {
            why = "the file's SHA-256 does not match the manifest";
            break;
        }
        LittleFS.remove(binPath(b).c_str());
        if (!LittleFS.rename(tmpPath(b).c_str(), binPath(b).c_str())) {
            why = "could not store the build";
            break;
        }
        saveNote(i, m);
        ok = true;
    } while (false);
    LittleFS.remove(tmpPath(b).c_str());
    Guard guard;
    working = false;
    if (ok) {
        loadStored(i);
        state = State::Idle;
        error.clear();
        Serial.printf("gauge firmware: %s %s stored\n", b.key, m.version.c_str());
    } else {
        state = State::Error;
        error = why;
        Serial.printf("gauge firmware: %s\n", why.c_str());
    }
}

}  // namespace

const Board* boards(size_t& count) {
    count = kBoardCount;
    return kBoards;
}

const Board* boardById(uint8_t id) {
    const int i = indexOf(id);
    return i < 0 ? nullptr : &kBoards[i];
}

void begin() {
    mtx = xSemaphoreCreateMutex();
    if (!LittleFS.exists(kDir)) LittleFS.mkdir(kDir);
    for (size_t i = 0; i < kBoardCount; i++) {
        LittleFS.remove(tmpPath(kBoards[i]).c_str());  // a fetch cut short
        loadStored(i);
    }
}

bool stored(uint8_t boardId, Build& out) {
    const int i = indexOf(boardId);
    if (i < 0) return false;
    Guard guard;
    if (!slots[i].haveStored) return false;
    out = slots[i].stored;
    return true;
}

bool newerThan(uint8_t boardId, uint8_t major, uint8_t minor, uint8_t patch) {
    Build b;
    if (!stored(boardId, b)) return false;
    char current[16];
    snprintf(current, sizeof(current), "%u.%u.%u", major, minor, patch);
    return hg::update::newerThan(b.version.c_str(), current);
}

void service(uint32_t nowMs, bool online) {
    (void)nowMs;
    bool wantCheck = false;
    int wantFetch = -1;
    {
        Guard guard;
        wantCheck = checkWanted;
        wantFetch = fetchWanted;
        checkWanted = false;
        fetchWanted = -1;
    }
    if (!wantCheck && wantFetch < 0) return;
    if (!online) {
        setState(State::Error, "join the home network first");
        return;
    }
    if (wantCheck) check();
    if (wantFetch >= 0) fetch(size_t(wantFetch));
}

bool busy() {
    Guard guard;
    return working;
}

void requestCheck() {
    Guard guard;
    checkWanted = true;
}

bool requestFetch(uint8_t boardId, std::string& err) {
    const int i = indexOf(boardId);
    if (i < 0) {
        err = "unknown gauge board";
        return false;
    }
    Guard guard;
    if (working) {
        err = "a fetch is already running";
        return false;
    }
    if (!slots[i].haveLatest) {
        err = "the site has no build for this board: check first";
        return false;
    }
    fetchWanted = i;
    return true;
}

bool remove(uint8_t boardId) {
    const int i = indexOf(boardId);
    if (i < 0) return false;
    Guard guard;
    if (working) return false;
    LittleFS.remove(binPath(kBoards[i]).c_str());
    LittleFS.remove(notePath(kBoards[i]).c_str());
    slots[i].haveStored = false;
    return true;
}

void statusJson(JsonObject out) {
    Guard guard;
    JsonArray list = out["boards"].to<JsonArray>();
    for (size_t i = 0; i < kBoardCount; i++) {
        JsonObject o = list.add<JsonObject>();
        o["id"] = kBoards[i].id;
        o["key"] = kBoards[i].key;
        o["name"] = kBoards[i].name;
        if (slots[i].haveStored) {
            JsonObject s = o["stored"].to<JsonObject>();
            s["version"] = slots[i].stored.version;
            s["size"] = slots[i].stored.size;
        } else {
            o["stored"] = nullptr;
        }
        if (slots[i].haveLatest) {
            JsonObject l = o["latest"].to<JsonObject>();
            l["version"] = slots[i].latest.version;
            l["size"] = slots[i].latest.size;
            l["date"] = slots[i].latest.date;
            l["notes"] = slots[i].latest.notes;
        } else {
            o["latest"] = nullptr;
        }
    }
    out["state"] = stateName(state);
    out["progress"] = progress;
    out["error"] = error;
    out["checked_s"] = everChecked ? int((millis() - checkedMs) / 1000) : -1;
}

}  // namespace gauge_firmware
