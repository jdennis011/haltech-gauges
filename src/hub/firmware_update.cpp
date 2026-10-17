#include "firmware_update.h"

#include <Arduino.h>
#include <Preferences.h>
#include <Update.h>
#include <esp_crt_bundle.h>
#include <esp_http_client.h>
#include <esp_ota_ops.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "hub_config.h"
#include "sd_card.h"
#include "update_manifest.h"

using hg::update::Manifest;

// The core marks a freshly installed build as good the moment it starts
// unless this says to wait; poll() marks it once it has run for a while, so a
// build that crashes early is rolled back by the bootloader.
extern "C" bool verifyRollbackLater() { return true; }

namespace firmware_update {

namespace {

const char* kPrefs = "update";
const uint32_t kFirstCheckMs = 30 * 1000;
const uint32_t kCheckEveryMs = 24u * 60u * 60u * 1000u;
const uint32_t kRetryCheckMs = 10 * 60 * 1000;
// A new build that has run this long without restarting is kept. Short, so
// a key-off soon after an update does not undo it.
const uint32_t kConfirmAfterMs = 10 * 1000;
const uint32_t kRestartDelayMs = 1500;
// TLS needs about 45 KB while a request runs.
const uint32_t kMinHeap = 96 * 1024;
const size_t kChunk = 4096;
const size_t kMaxManifest = 8 * 1024;

enum class State : uint8_t { Idle, Checking, UpToDate, Available, Downloading, Installing, Restarting, Error };

const char* stateName(State s) {
    switch (s) {
        case State::Idle: return "idle";
        case State::Checking: return "checking";
        case State::UpToDate: return "up to date";
        case State::Available: return "available";
        case State::Downloading: return "downloading";
        case State::Installing: return "installing";
        case State::Restarting: return "restarting";
        case State::Error: return "error";
    }
    return "idle";
}

SemaphoreHandle_t mtx = nullptr;
// Under mtx.
State state = State::Idle;
std::string error;
std::string last;  // what the previous update did, for this boot
Manifest available;
bool haveAvailable = false;
uint32_t progress = 0;  // percent of the download or upload
uint32_t checkedMs = 0;
bool everChecked = false;
bool checkWanted = false;
bool installWanted = false;
bool writing = false;  // the spare slot is being written
uint32_t restartAtMs = 0;
bool restartWanted = false;
// The task's own.
uint32_t nextCheckMs = kFirstCheckMs;
bool pendingVerify = false;
bool hasSlot = false;
size_t uploadTotal = 0;
size_t uploadDone = 0;

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

void setState(State s, const char* err = nullptr) {
    Guard guard;
    state = s;
    if (err) error = err;
}

void setProgress(size_t done, size_t total) {
    Guard guard;
    progress = total ? uint32_t(uint64_t(done) * 100 / total) : 0;
}

void noteInstall(const char* to) {
    Preferences p;
    p.begin(kPrefs, false);
    p.putString("from", HUB_FW_VERSION);
    p.putString("to", to);
    p.end();
}

void scheduleRestart() {
    Guard guard;
    restartWanted = true;
    restartAtMs = millis() + kRestartDelayMs;
}

// ------------------------------------------------------------------ HTTP

typedef bool (*Sink)(const uint8_t* data, size_t len, void* ctx);

// GET `url`, handing the body to `sink` piece by piece. Returns the HTTP
// status, or -1 with the reason in `why` when no reply came. `length` is the
// Content-Length, or -1 if the server did not say.
int httpGet(const char* url, Sink sink, void* ctx, std::string& why, int64_t& length) {
    length = -1;
    esp_http_client_config_t cfg = {};
    cfg.url = url;
    cfg.method = HTTP_METHOD_GET;
    cfg.timeout_ms = 30000;
    cfg.crt_bundle_attach = esp_crt_bundle_attach;
    cfg.buffer_size = 4096;
    cfg.buffer_size_tx = 1024;
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) {
        why = "could not start a request";
        return -1;
    }
    esp_http_client_set_header(c, "User-Agent", "haltech-hub/" HUB_FW_VERSION);
    const esp_err_t err = esp_http_client_open(c, 0);
    if (err != ESP_OK) {
        why = std::string("connect: ") + esp_err_to_name(err);
        esp_http_client_cleanup(c);
        return -1;
    }
    length = esp_http_client_fetch_headers(c);
    if (length < 0) {
        why = "no reply";
        esp_http_client_close(c);
        esp_http_client_cleanup(c);
        return -1;
    }
    if (esp_http_client_is_chunked_response(c)) length = -1;
    const int status = esp_http_client_get_status_code(c);
    std::vector<uint8_t> buf(kChunk);
    bool ok = true;
    while (ok) {
        const int n = esp_http_client_read(c, reinterpret_cast<char*>(buf.data()), int(buf.size()));
        if (n < 0) {
            why = "the download broke off";
            ok = false;
            break;
        }
        if (n == 0) break;
        if (status >= 200 && status < 300) ok = sink(buf.data(), size_t(n), ctx);
    }
    esp_http_client_close(c);
    esp_http_client_cleanup(c);
    return ok ? status : -1;
}

bool toString(const uint8_t* data, size_t len, void* ctx) {
    std::string* s = static_cast<std::string*>(ctx);
    if (s->size() + len > kMaxManifest) return false;
    s->append(reinterpret_cast<const char*>(data), len);
    return true;
}

std::string siteUrl(const char* file) {
    return std::string(HUB_CLOUD_URL) + "/fw/" + board() + "/" + file;
}

// ----------------------------------------------------------------- check

void check() {
    setState(State::Checking, "");
    std::string body, why;
    int64_t length;
    const int status = httpGet(siteUrl("manifest.json").c_str(), toString, &body, why, length);
    {
        Guard guard;
        checkedMs = millis();
        everChecked = true;
    }
    if (status < 0) {
        if (why.empty()) why = "the manifest is too large";
        setState(State::Error, why.c_str());
        return;
    }
    if (status == 404) {
        Guard guard;
        haveAvailable = false;
        state = State::UpToDate;
        error = "the site has no build for this board yet";
        return;
    }
    if (status != 200) {
        char buf[40];
        snprintf(buf, sizeof(buf), "the site answered HTTP %d", status);
        setState(State::Error, buf);
        return;
    }
    Manifest m;
    if (!hg::update::parseManifest(reinterpret_cast<const uint8_t*>(body.data()), body.size(), board(), m, why)) {
        setState(State::Error, why.c_str());
        return;
    }
    Guard guard;
    error.clear();
    if (hg::update::newerThan(m.version.c_str(), HUB_FW_VERSION)) {
        available = m;
        haveAvailable = true;
        state = State::Available;
    } else {
        haveAvailable = false;
        state = State::UpToDate;
    }
}

// --------------------------------------------------------------- install

struct Download {
    mbedtls_sha256_context sha;
    size_t written = 0;
    size_t expected = 0;
    std::string why;
};

bool toSlot(const uint8_t* data, size_t len, void* ctx) {
    Download* d = static_cast<Download*>(ctx);
    if (d->written + len > d->expected) {
        d->why = "the file is larger than the manifest says";
        return false;
    }
    if (Update.write(const_cast<uint8_t*>(data), len) != len) {
        d->why = Update.errorString();
        return false;
    }
    mbedtls_sha256_update(&d->sha, data, len);
    d->written += len;
    setProgress(d->written, d->expected);
    return true;
}

void install() {
    Manifest m;
    {
        Guard guard;
        if (!haveAvailable) {
            state = State::Error;
            error = "nothing to install: check for an update first";
            return;
        }
        if (writing) return;
        m = available;
        writing = true;
    }
    bool ok = false;
    std::string why;
    do {
        if (!hasSlot) {
            why = "this board has no spare app slot";
            break;
        }
        if (ESP.getFreeHeap() < kMinHeap) {
            why = "not enough free memory; try again in a moment";
            break;
        }
        setState(State::Downloading, "");
        setProgress(0, m.size);
        if (!Update.begin(m.size)) {
            why = Update.errorString();
            break;
        }
        Download d;
        d.expected = m.size;
        mbedtls_sha256_init(&d.sha);
        mbedtls_sha256_starts(&d.sha, 0);
        int64_t length;
        const int status = httpGet(siteUrl("firmware.bin").c_str(), toSlot, &d, d.why, length);
        uint8_t digest[32];
        mbedtls_sha256_finish(&d.sha, digest);
        mbedtls_sha256_free(&d.sha);
        if (status < 0) {
            why = d.why;
            Update.abort();
            break;
        }
        if (status != 200) {
            char buf[40];
            snprintf(buf, sizeof(buf), "the site answered HTTP %d", status);
            why = buf;
            Update.abort();
            break;
        }
        if (d.written != m.size) {
            char buf[80];
            snprintf(buf, sizeof(buf), "the download was %u of %u bytes", unsigned(d.written), unsigned(m.size));
            why = buf;
            Update.abort();
            break;
        }
        char hex[65];
        for (int i = 0; i < 32; i++) snprintf(hex + 2 * i, 3, "%02x", digest[i]);
        if (m.sha256 != hex) {
            why = "the file's SHA-256 does not match the manifest";
            Update.abort();
            break;
        }
        setState(State::Installing);
        if (!Update.end()) {
            why = Update.errorString();
            break;
        }
        ok = true;
    } while (false);

    if (!ok) {
        Serial.printf("update: %s\n", why.c_str());
        Guard guard;
        writing = false;
        state = State::Error;
        error = why;
        return;
    }
    noteInstall(m.version.c_str());
    Serial.printf("update: %s installed, restarting\n", m.version.c_str());
    {
        Guard guard;
        writing = false;
        state = State::Restarting;
        error.clear();
    }
    scheduleRestart();
}

}  // namespace

const char* board() {
#if CONFIG_IDF_TARGET_ESP32C6
    return "esp32c6";
#elif CONFIG_IDF_TARGET_ESP32P4
    return "esp32p4";
#elif CONFIG_IDF_TARGET_ESP32C3
    return "esp32c3";
#elif CONFIG_IDF_TARGET_ESP32S3
    return "esp32s3";
#else
    return "esp32";
#endif
}

void begin() {
    mtx = xSemaphoreCreateMutex();
    hasSlot = esp_ota_get_next_update_partition(nullptr) != nullptr;
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t imgState;
    if (running && esp_ota_get_state_partition(running, &imgState) == ESP_OK &&
        imgState == ESP_OTA_IMG_PENDING_VERIFY) {
        pendingVerify = true;
    }

    // What the last install came to: this is the new build's first start, or
    // the old build back after the new one failed to run.
    Preferences p;
    p.begin(kPrefs, false);
    const String from = p.getString("from", "");
    const String to = p.getString("to", "");
    if (from.length()) {
        if (pendingVerify) {
            last = std::string("updated from ") + from.c_str() + " at this start";
            Serial.printf("Firmware updated from %s to " HUB_FW_VERSION "\n", from.c_str());
        } else {
            last = std::string("the update to ") + to.c_str() + " did not run and was undone; still on " HUB_FW_VERSION;
            Serial.printf("Firmware update to %s was rolled back\n", to.c_str());
        }
        p.remove("from");
        p.remove("to");
    }
    p.end();
    if (!hasSlot) Serial.println("No spare app slot: firmware updates over Wi-Fi are not possible on this board");
}

void poll(uint32_t nowMs) {
    if (pendingVerify && nowMs >= kConfirmAfterMs) {
        pendingVerify = false;
        if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
            Serial.println("Firmware " HUB_FW_VERSION " confirmed");
        }
    }
    bool restart = false;
    {
        Guard guard;
        restart = restartWanted && int32_t(nowMs - restartAtMs) >= 0;
    }
    if (restart) {
        Serial.println("Restarting into the new firmware");
        Serial.flush();
        sd_card::forgetProbe();  // a deliberate restart, not a card that crashed the hub
        ESP.restart();
    }
}

void service(uint32_t nowMs, bool online) {
    bool wantCheck = false, wantInstall = false;
    {
        Guard guard;
        wantCheck = checkWanted;
        wantInstall = installWanted;
        checkWanted = installWanted = false;
    }
    if (wantInstall) {
        if (!online) {
            setState(State::Error, "join the home network first");
        } else {
            install();
        }
        return;
    }
    const bool due = int32_t(nowMs - nextCheckMs) >= 0;
    if (!wantCheck && !due) return;
    if (!online) {
        if (wantCheck) setState(State::Error, "join the home network first");
        nextCheckMs = nowMs + kRetryCheckMs;
        return;
    }
    if (ESP.getFreeHeap() < kMinHeap) {
        if (wantCheck) setState(State::Error, "not enough free memory; try again in a moment");
        nextCheckMs = nowMs + 60 * 1000;
        return;
    }
    check();
    bool failed;
    {
        Guard guard;
        failed = state == State::Error;
    }
    nextCheckMs = nowMs + (failed ? kRetryCheckMs : kCheckEveryMs);
}

bool busy() {
    Guard guard;
    return writing || restartWanted;
}

void requestCheck() {
    Guard guard;
    if (writing || restartWanted) return;
    checkWanted = true;
}

bool requestInstall(std::string& err) {
    Guard guard;
    if (!hasSlot) {
        err = "this board has no spare app slot";
        return false;
    }
    if (writing || restartWanted) {
        err = "an update is already running";
        return false;
    }
    if (!haveAvailable) {
        err = "nothing to install: check for an update first";
        return false;
    }
    installWanted = true;
    return true;
}

bool uploadBegin(size_t total, std::string& err) {
    {
        Guard guard;
        if (!hasSlot) {
            err = "this board has no spare app slot";
            return false;
        }
        if (writing || restartWanted) {
            err = "an update is already running";
            return false;
        }
        if (total < 64 * 1024) {
            err = "that is too small to be a firmware image";
            return false;
        }
        writing = true;
        state = State::Installing;
        error.clear();
        progress = 0;
    }
    uploadTotal = total;
    uploadDone = 0;
    if (!Update.begin(total)) {
        err = Update.errorString();
        Guard guard;
        writing = false;
        state = State::Error;
        error = err;
        return false;
    }
    return true;
}

bool uploadWrite(const uint8_t* data, size_t len) {
    if (Update.write(const_cast<uint8_t*>(data), len) != len) {
        const std::string why = Update.errorString();
        Update.abort();
        Guard guard;
        writing = false;
        state = State::Error;
        error = why;
        return false;
    }
    uploadDone += len;
    setProgress(uploadDone, uploadTotal);
    return true;
}

bool uploadEnd(std::string& err) {
    if (!Update.end()) {
        err = Update.errorString();
        Guard guard;
        writing = false;
        state = State::Error;
        error = err;
        return false;
    }
    noteInstall("a file uploaded from the page");
    Serial.println("update: uploaded image installed, restarting");
    {
        Guard guard;
        writing = false;
        state = State::Restarting;
        error.clear();
    }
    scheduleRestart();
    return true;
}

void uploadAbort() {
    if (Update.isRunning()) Update.abort();
    Guard guard;
    if (writing) {
        writing = false;
        state = State::Error;
        if (error.empty()) error = "the upload was cut short";
    }
}

int fetch(const char* url, HttpSink sink, void* ctx, std::string& why, int64_t& length) {
    return httpGet(url, sink, ctx, why, length);
}

void statusJson(JsonObject out) {
    Guard guard;
    out["current"] = HUB_FW_VERSION;
    out["board"] = board();
    out["slot"] = hasSlot;
    out["state"] = stateName(state);
    out["progress"] = progress;
    out["error"] = error;
    out["last"] = last;
    out["confirmed"] = !pendingVerify;  // false only in a new build's first seconds
    out["checked_s"] = everChecked ? int((millis() - checkedMs) / 1000) : -1;
    if (haveAvailable) {
        JsonObject a = out["available"].to<JsonObject>();
        a["version"] = available.version;
        a["size"] = available.size;
        a["date"] = available.date;
        a["notes"] = available.notes;
    } else {
        out["available"] = nullptr;
    }
}

}  // namespace firmware_update
