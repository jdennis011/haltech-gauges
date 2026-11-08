#include "datalog.h"

#include <Arduino.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <algorithm>
#include <atomic>
#include <vector>

#include "haltech.h"
#include "sd_card.h"

using namespace hg;

namespace datalog {

namespace {

const char* kDir = "/logs";
const char* kPrefs = "hub";
const uint32_t kPeriodMs = 1000 / kRateHz;
const uint32_t kFlushMs = 1000;
const uint32_t kIdleCloseMs = 5000;      // no live data this long: the drive is over
const uint32_t kRotateBytes = 64u * 1024u * 1024u;
const uint32_t kRetryOpenMs = 30000;     // after a file could not be opened
const uint64_t kKeepFreeBytes = 16ull * 1024 * 1024;
const uint32_t kSpaceRefreshMs = 60000;
const uint32_t kListRefreshMs = 10000;

const ChannelStore* live = nullptr;
std::atomic<bool> on{true};
std::atomic<bool> mountWanted{false};
std::atomic<bool> mounting{false};
std::atomic<bool> formatWanted{false};
std::atomic<bool> formatRunning{false};
std::atomic<uint8_t> formatKind{0};
std::atomic<bool> formatOk{false};
std::atomic<bool> writing{false};
std::atomic<bool> paused{false};  // the card stopped taking writes; off until the next Mount
std::atomic<uint32_t> rows{0};
std::atomic<uint32_t> bytes{0};
std::atomic<uint32_t> startMs{0};

// The task's own.
File file;
std::string fileName;
uint32_t lastRowMs = 0, lastFlushMs = 0, lastDataMs = 0, openFailedMs = 0, lastSpaceMs = 0, lastListMs = 0;
uint8_t openFailures = 0;
bool openFailed = false;
std::string row;

// The file's name and the directory listing, for the status, under a lock
// since the task changes them.
SemaphoreHandle_t mtx = nullptr;
char nameShown[48] = "";
char errorText[96] = "";
char formatResult[160] = "";  // how the last format went, for the page
struct Entry {
    std::string name;
    uint32_t bytes;
};
std::vector<Entry> listing;
// Lines for other files (appendLine), waiting for the task.
struct Pending {
    std::string file;
    std::string line;
    const char* header;
};
std::vector<Pending> pending;

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

void setNameShown(const char* name) {
    Guard guard;
    snprintf(nameShown, sizeof(nameShown), "%s", name ? name : "");
}

void setError(const char* text) {
    Guard guard;
    snprintf(errorText, sizeof(errorText), "%s", text ? text : "");
}

// The task reads the directory; the web shows what it read.
void refreshListing() {
    fs::FS* fs = sd_card::fs();
    std::vector<Entry> fresh;
    if (fs) {
        File dir = fs->open(kDir);
        if (dir && dir.isDirectory()) {
            for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
                if (!f.isDirectory()) fresh.push_back({f.name(), uint32_t(f.size())});
                f.close();
            }
            dir.close();
        }
    }
    std::sort(fresh.begin(), fresh.end(), [](const Entry& a, const Entry& b) { return a.name > b.name; });
    Guard guard;
    listing.swap(fresh);
}

bool anyLive(uint32_t nowMs) {
    for (uint16_t i = 0; i < CH_COUNT; i++) {
        float v;
        if (live->get(ChannelId(i), nowMs, v)) return true;
    }
    return false;
}

std::string newName() {
    const time_t now = time(nullptr);
    char buf[48];
    if (now > 1700000000) {
        struct tm lt;
        localtime_r(&now, &lt);
        strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S.csv", &lt);
    } else {
        Preferences p;
        p.begin(kPrefs, false);
        const uint32_t seq = p.getUInt("log_seq", 0) + 1;
        p.putUInt("log_seq", seq);
        p.end();
        snprintf(buf, sizeof(buf), "log-%04lu.csv", (unsigned long)seq);
    }
    return buf;
}

void closeFile() {
    if (!file) return;
    file.close();
    Serial.printf("datalog: %s closed, %lu rows\n", fileName.c_str(), (unsigned long)rows.load());
    fileName.clear();
    setNameShown("");
    writing = false;
}

bool writeHeader() {
    row = "time_ms";
    for (uint16_t i = 0; i < CH_COUNT; i++) {
        const ChannelDef& d = channelDef(ChannelId(i));
        row += ',';
        row += d.name;
        const char* u = unitSymbol(d.unit);
        if (u && u[0]) {
            row += " (";
            row += u;
            row += ')';
        }
    }
    row += '\n';
    const size_t wrote = file.print(row.c_str());
    bytes += wrote;
    return wrote == row.size();
}

bool openFile(uint32_t nowMs) {
    fs::FS* fs = sd_card::fs();
    if (!fs) return false;
    if (sd_card::freeBytes() < kKeepFreeBytes) {
        Serial.println("datalog: the card is full");
        return false;
    }
    if (!fs->exists(kDir)) fs->mkdir(kDir);
    fileName = newName();
    file = fs->open((std::string(kDir) + "/" + fileName).c_str(), FILE_WRITE);
    if (!file) {
        Serial.println("datalog: could not open a file on the card");
        fileName.clear();
        return false;
    }
    startMs = nowMs;
    lastRowMs = lastFlushMs = lastDataMs = nowMs;
    rows = 0;
    bytes = 0;
    if (!writeHeader()) {
        Serial.println("datalog: the card would not take the header");
        file.close();
        fs->remove((std::string(kDir) + "/" + fileName).c_str());
        fileName.clear();
        return false;
    }
    setNameShown(fileName.c_str());
    writing = true;
    Serial.printf("datalog: %s\n", fileName.c_str());
    return true;
}

void appendValue(float v) {
    char buf[24];
    if (v == float(int32_t(v)) && v > -1e9f && v < 1e9f) {
        snprintf(buf, sizeof(buf), "%ld", long(v));
    } else {
        snprintf(buf, sizeof(buf), "%.3f", double(v));
        char* end = buf + strlen(buf) - 1;
        while (end > buf && *end == '0') *end-- = 0;  // trailing zeros go
        if (*end == '.') *end = 0;
    }
    row += buf;
}

void writeRow(uint32_t nowMs) {
    row.clear();
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)(nowMs - startMs.load()));
    row += buf;
    for (uint16_t i = 0; i < CH_COUNT; i++) {
        row += ',';
        float v;
        if (live->get(ChannelId(i), nowMs, v)) appendValue(v);
    }
    row += '\n';
    const size_t wrote = file.print(row.c_str());
    bytes += wrote;
    rows++;
    if (wrote != row.size()) {
        Serial.println("datalog: write failed; the card may be full or gone");
        closeFile();
        openFailed = true;
        openFailedMs = nowMs;
        if (++openFailures >= 3) {
            // A card that keeps failing is left alone: hammering it holds the
            // filesystem's lock and makes the page wait on it.
            paused = true;
            setError("the card stopped taking writes; logging is paused until Mount is pressed");
            Serial.println("datalog: paused after repeated write failures");
        }
    }
}

// Writes the queued lines of other files, each opened for append and closed again.
void writePending() {
    std::vector<Pending> batch;
    {
        Guard g;
        batch.swap(pending);
    }
    fs::FS* fs = sd_card::fs();
    if (!fs || batch.empty()) return;
    if (!fs->exists(kDir)) fs->mkdir(kDir);
    for (const Pending& p : batch) {
        const std::string path = std::string(kDir) + "/" + p.file;
        const bool fresh = !fs->exists(path.c_str());
        File f = fs->open(path.c_str(), FILE_APPEND);
        if (!f) {
            Serial.printf("datalog: could not open %s\n", p.file.c_str());
            continue;
        }
        if (fresh && p.header) {
            f.print(p.header);
            f.print('\n');
        }
        f.print(p.line.c_str());
        f.print('\n');
        f.close();
    }
}

void step(uint32_t nowMs) {
    if (formatWanted.exchange(false)) {
        // The card is the task's: the file goes, then the format, then the
        // card comes back as at start (sd_card::format mounts it again).
        if (file) closeFile();
        formatRunning = true;
        char report[sizeof(formatResult)];
        const bool ok = sd_card::format(hg::sd::FsKind(formatKind.load()), report, sizeof(report));
        {
            Guard guard;
            snprintf(formatResult, sizeof(formatResult), "%s", report);
        }
        formatOk = ok;
        formatRunning = false;
        Serial.printf("datalog: %s\n", report);
        openFailed = false;
        openFailures = 0;
        paused = false;
        setError("");
        refreshListing();
    }
    if (mountWanted.exchange(false)) {
        mounting = true;
        sd_card::begin(true);
        mounting = false;
        openFailed = false;
        openFailures = 0;
        paused = false;
        setError("");
        refreshListing();
    }
    if (sd_card::mounted() && !paused) writePending();
    if (!on || !sd_card::mounted() || paused) {
        if (file) closeFile();
        return;
    }
    if (uint32_t(nowMs - lastSpaceMs) >= kSpaceRefreshMs) {
        lastSpaceMs = nowMs;
        sd_card::refreshSpace();
    }
    if (!file && uint32_t(nowMs - lastListMs) >= kListRefreshMs) {
        lastListMs = nowMs;
        refreshListing();
    }
    const bool isLive = anyLive(nowMs);
    if (isLive) lastDataMs = nowMs;
    if (!file) {
        if (!isLive) return;
        if (openFailed && uint32_t(nowMs - openFailedMs) < kRetryOpenMs) return;
        if (!openFile(nowMs)) {
            openFailed = true;
            openFailedMs = nowMs;
            if (++openFailures >= 3) {
                paused = true;
                setError("the card would not take a new file; logging is paused until Mount is pressed");
            }
        } else {
            refreshListing();
        }
        return;
    }
    if (uint32_t(nowMs - lastRowMs) < kPeriodMs) return;
    lastRowMs = nowMs;
    if (!isLive && uint32_t(nowMs - lastDataMs) >= kIdleCloseMs) {
        closeFile();
        return;
    }
    writeRow(nowMs);
    if (!file) return;
    if (uint32_t(nowMs - lastFlushMs) >= kFlushMs) {
        lastFlushMs = nowMs;
        file.flush();
    }
    if (bytes >= kRotateBytes) {
        closeFile();
        openFile(nowMs);
    }
}

void task(void*) {
    // The card first, off the main loop: a slot that stalls stalls only this.
    mounting = true;
    sd_card::begin();
    mounting = false;
    refreshListing();
    for (;;) {
        step(millis());
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

}  // namespace

void begin(const ChannelStore& store) {
    live = &store;
    mtx = xSemaphoreCreateMutex();
    Preferences p;
    p.begin(kPrefs, true);
    on = p.getBool("log_on", true);
    p.end();
    xTaskCreate(task, "datalog", 8192, nullptr, 2, nullptr);
}

bool enabled() { return on; }

void setEnabled(bool enable) {
    on = enable;
    Preferences p;
    p.begin(kPrefs, false);
    p.putBool("log_on", enable);
    p.end();
}

bool active() { return writing; }

void requestMount() { mountWanted = true; }

void requestFormat(hg::sd::FsKind kind) {
    formatKind = uint8_t(kind);
    formatWanted = true;
}

bool formatting() { return formatWanted.load() || formatRunning.load(); }

void appendLine(const char* fileName, const std::string& line, const char* header) {
    if (!mtx || !sd_card::mounted()) return;
    Guard g;
    if (pending.size() < 32) pending.push_back({fileName, line, header});
}

bool validName(const char* name) {
    if (!name || !name[0] || strlen(name) > 40) return false;
    for (const char* c = name; *c; c++) {
        if (!isalnum((unsigned char)*c) && *c != '-' && *c != '_' && *c != '.') return false;
    }
    return strstr(name, "..") == nullptr;
}

std::string path(const char* name) { return std::string(kDir) + "/" + name; }

bool remove(const char* name) {
    fs::FS* fs = sd_card::fs();
    if (!fs || !validName(name)) return false;
    {
        Guard guard;
        if (writing && strcmp(nameShown, name) == 0) return false;  // being written
    }
    return fs->remove(path(name).c_str());
}

void statusJson(JsonObject out) {
    out["enabled"] = on.load();
    out["active"] = writing.load();
    out["mounting"] = mounting.load();
    out["formatting"] = formatting();
    out["format_ok"] = formatOk.load();
    out["paused"] = paused.load();
    {
        Guard guard;
        out["file"] = nameShown;
        out["error"] = errorText;
        out["format_result"] = formatResult;
    }
    out["rows"] = rows.load();
    out["bytes"] = bytes.load();
    out["seconds"] = writing ? (millis() - startMs.load()) / 1000 : 0;
}

void listJson(JsonArray out) {
    if (!sd_card::mounted()) return;
    Guard guard;
    for (const Entry& e : listing) {
        JsonObject o = out.add<JsonObject>();
        o["name"] = e.name;
        o["bytes"] = e.bytes;
        o["writing"] = writing && e.name == nameShown;
    }
}

}  // namespace datalog
