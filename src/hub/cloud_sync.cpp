#include "cloud_sync.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_crt_bundle.h>
#include <esp_http_client.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "alert_monitor.h"
#include "hub_config.h"
#include "hub_state.h"
#include "image_store.h"
#include "library_store.h"
#include "theme_colours.h"

namespace cloud_sync {

namespace {

const char* kPrefs = "cloud";
const uint32_t kPollMs = 60 * 1000;
const uint32_t kStatusMs = 5 * 60 * 1000;
const uint32_t kRetryMs = 20 * 1000;
// TLS needs about 45 KB while a request runs; below this the web page and the
// bus come first.
const uint32_t kMinHeap = 96 * 1024;
const size_t kMaxReply = 72 * 1024;  // a config with the Worker's slack, or the manifest

struct Change {
    Kind kind;
    std::string name;
    bool deleted;
};

SemaphoreHandle_t mtx = nullptr;
std::vector<Change> pending;  // under mtx
std::string linkCode;         // under mtx
bool unlinkWanted = false;    // under mtx
bool syncWanted = false;      // under mtx

// The task's own.
std::string token, email, hubName, hubId;
uint32_t appliedRev = 0, appliedThemeRev = 0, appliedAlertsRev = 0;
std::string state = "not linked";
std::string lastError;
uint32_t lastSyncMs = 0;
bool everSynced = false;
uint32_t nextPollMs = 0, nextStatusMs = 0;
TaskHandle_t task = nullptr;

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

void setState(const char* s, const char* error = nullptr) {
    Guard guard;
    state = s;
    if (error) lastError = error;
}

void loadPrefs() {
    Preferences p;
    p.begin(kPrefs, true);
    token = p.getString("token", "").c_str();
    email = p.getString("email", "").c_str();
    hubName = p.getString("name", HUB_HOSTNAME).c_str();
    appliedRev = p.getUInt("rev", 0);
    appliedThemeRev = p.getUInt("tcrev", 0);
    appliedAlertsRev = p.getUInt("alrev", 0);
    p.end();
}

void saveRevs() {
    Preferences p;
    p.begin(kPrefs, false);
    p.putUInt("rev", appliedRev);
    p.putUInt("tcrev", appliedThemeRev);
    p.putUInt("alrev", appliedAlertsRev);
    p.end();
}

void saveLink() {
    Preferences p;
    p.begin(kPrefs, false);
    p.putString("token", token.c_str());
    p.putString("email", email.c_str());
    p.putString("name", hubName.c_str());
    p.end();
}

// ----------------------------------------------------------------- HTTP

struct Reply {
    int status = 0;
    std::string body;
    bool ok() const { return status >= 200 && status < 300; }
};

// One request to the site. The body comes from `body` or `bodyFile`; the
// reply goes to `out` (up to maxOut) or `outFile`. False on a transport
// failure, with the reason in lastError; an HTTP error is in Reply::status.
bool request(esp_http_client_method_t method, const char* path, const std::string* body, File* bodyFile,
             const char* contentType, Reply& reply, size_t maxOut = kMaxReply, File* outFile = nullptr) {
    reply = Reply();
    const std::string url = std::string(HUB_CLOUD_URL) + path;
    esp_http_client_config_t cfg = {};
    cfg.url = url.c_str();
    cfg.method = method;
    cfg.timeout_ms = 20000;
    cfg.crt_bundle_attach = esp_crt_bundle_attach;
    cfg.buffer_size = 2048;
    cfg.buffer_size_tx = 1024;
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) {
        lastError = "could not start a request";
        return false;
    }
    if (!token.empty()) {
        const std::string bearer = "Bearer " + token;
        esp_http_client_set_header(c, "Authorization", bearer.c_str());
    }
    if (contentType) esp_http_client_set_header(c, "Content-Type", contentType);
    esp_http_client_set_header(c, "User-Agent", "haltech-hub/" HUB_FW_VERSION);

    const int length = body ? int(body->size()) : (bodyFile ? int(bodyFile->size()) : 0);
    esp_err_t err = esp_http_client_open(c, length);
    if (err != ESP_OK) {
        lastError = std::string("connect: ") + esp_err_to_name(err);
        esp_http_client_cleanup(c);
        return false;
    }
    bool sent = true;
    if (body) {
        sent = esp_http_client_write(c, body->data(), int(body->size())) == int(body->size());
    } else if (bodyFile) {
        uint8_t buf[1024];
        bodyFile->seek(0);
        while (sent) {
            const size_t n = bodyFile->read(buf, sizeof(buf));
            if (n == 0) break;
            sent = esp_http_client_write(c, reinterpret_cast<const char*>(buf), int(n)) == int(n);
        }
    }
    if (!sent) {
        lastError = "send failed";
        esp_http_client_close(c);
        esp_http_client_cleanup(c);
        return false;
    }
    if (esp_http_client_fetch_headers(c) < 0) {
        lastError = "no reply";
        esp_http_client_close(c);
        esp_http_client_cleanup(c);
        return false;
    }
    reply.status = esp_http_client_get_status_code(c);
    char buf[1024];
    bool fits = true;
    for (;;) {
        const int n = esp_http_client_read(c, buf, sizeof(buf));
        if (n <= 0) break;
        if (outFile && reply.ok()) {
            if (outFile->write(reinterpret_cast<const uint8_t*>(buf), size_t(n)) != size_t(n)) fits = false;
        } else if (reply.body.size() + size_t(n) <= maxOut) {
            reply.body.append(buf, size_t(n));
        } else {
            fits = false;
        }
    }
    esp_http_client_close(c);
    esp_http_client_cleanup(c);
    if (!fits) {
        lastError = "reply too large";
        return false;
    }
    return true;
}

// A JSON error message from the site, or the status code.
std::string httpError(const Reply& r) {
    JsonDocument doc;
    if (!deserializeJson(doc, r.body) && doc["error"].is<const char*>()) return doc["error"].as<const char*>();
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP %d", r.status);
    return buf;
}

// ------------------------------------------------------------- the items

bool downloadConfig(const char* name) {
    Reply r;
    if (!request(HTTP_METHOD_GET, (std::string("/api/configs/") + name).c_str(), nullptr, nullptr, nullptr, r)) return false;
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    std::string err;
    if (!library_store::put(name, reinterpret_cast<const uint8_t*>(r.body.data()), r.body.size(), err)) {
        lastError = std::string(name) + ": " + err;
        return false;
    }
    Serial.printf("cloud: config %s down\n", name);
    return true;
}

bool uploadConfig(const char* name) {
    std::string text;
    if (!library_store::read(name, text)) return true;  // gone meanwhile
    Reply r;
    if (!request(HTTP_METHOD_PUT, (std::string("/api/configs/") + name).c_str(), &text, nullptr, "application/json", r)) return false;
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    Serial.printf("cloud: config %s up\n", name);
    return true;
}

bool deleteRemote(const char* area, const char* name) {
    Reply r;
    if (!request(HTTP_METHOD_DELETE, (std::string("/api/") + area + "/" + name).c_str(), nullptr, nullptr, nullptr, r)) return false;
    if (!r.ok() && r.status != 404) {
        lastError = httpError(r);
        return false;
    }
    return true;
}

bool downloadImage(const char* name) {
    File f = LittleFS.open(image_store::uploadPath(), "w");
    if (!f) {
        lastError = "could not write to storage";
        return false;
    }
    Reply r;
    const bool got = request(HTTP_METHOD_GET, (std::string("/api/images/") + name).c_str(), nullptr, nullptr, nullptr, r, 0, &f);
    f.close();
    if (!got || !r.ok()) {
        LittleFS.remove(image_store::uploadPath());
        if (got) lastError = httpError(r);
        return false;
    }
    std::string err;
    if (!image_store::accept(name, err)) {
        lastError = std::string(name) + ": " + err;
        return false;
    }
    Serial.printf("cloud: image %s down\n", name);
    return true;
}

bool uploadImage(const char* name) {
    std::string path;
    const char* type = nullptr;
    if (!image_store::file(name, path, type)) return true;  // gone meanwhile
    File f = LittleFS.open(path.c_str(), "r");
    if (!f) return true;
    Reply r;
    const bool sent = request(HTTP_METHOD_PUT, (std::string("/api/images/") + name).c_str(), nullptr, &f, type, r);
    f.close();
    if (!sent) return false;
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    Serial.printf("cloud: image %s up\n", name);
    return true;
}

// A settings item (theme colours or alerts) as the site keeps it: the bare list.
bool uploadSettings(Kind kind) {
    JsonDocument doc;
    if (kind == Kind::ThemeColours) {
        theme_colours::json(doc.to<JsonArray>());
    } else {
        alert_monitor::rulesJson(doc.to<JsonArray>());
    }
    std::string body;
    serializeJson(doc, body);
    const char* path = kind == Kind::ThemeColours ? "/api/settings/themecolours" : "/api/settings/alerts";
    Reply r;
    if (!request(HTTP_METHOD_PUT, path, &body, nullptr, "application/json", r)) return false;
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    // The site says which revision this made, so the next poll does not bring it back down.
    JsonDocument reply;
    if (!deserializeJson(reply, r.body)) {
        const uint32_t rev = reply["settingsRev"] | 0;
        if (kind == Kind::ThemeColours) appliedThemeRev = rev; else appliedAlertsRev = rev;
        saveRevs();
    }
    Serial.printf("cloud: %s up\n", kind == Kind::ThemeColours ? "theme colours" : "alerts");
    return true;
}

bool downloadSettings(Kind kind, uint32_t rev) {
    const char* path = kind == Kind::ThemeColours ? "/api/settings/themecolours" : "/api/settings/alerts";
    Reply r;
    if (!request(HTTP_METHOD_GET, path, nullptr, nullptr, nullptr, r)) return false;
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    if (r.body == "null" || r.body.empty()) {
        // The account has none yet: the hub's own go up instead.
        return uploadSettings(kind);
    }
    std::string err;
    const bool ok = kind == Kind::ThemeColours
                        ? theme_colours::set(reinterpret_cast<const uint8_t*>(r.body.data()), r.body.size(), err)
                        : alert_monitor::setRules(reinterpret_cast<const uint8_t*>(r.body.data()), r.body.size(), err);
    if (!ok) {
        lastError = err;
        return false;
    }
    if (kind == Kind::ThemeColours) appliedThemeRev = rev; else appliedAlertsRev = rev;
    saveRevs();
    Serial.printf("cloud: %s down\n", kind == Kind::ThemeColours ? "theme colours" : "alerts");
    return true;
}

// Sends up what the hub's own page changed. False on the first failure, to
// try again later with the rest still queued.
bool pushPending() {
    for (;;) {
        Change ch;
        {
            Guard guard;
            if (pending.empty()) return true;
            ch = pending.front();
        }
        bool ok;
        switch (ch.kind) {
            case Kind::Config: ok = ch.deleted ? deleteRemote("configs", ch.name.c_str()) : uploadConfig(ch.name.c_str()); break;
            case Kind::Image: ok = ch.deleted ? deleteRemote("images", ch.name.c_str()) : uploadImage(ch.name.c_str()); break;
            default: ok = uploadSettings(ch.kind); break;
        }
        if (!ok) return false;
        Guard guard;
        if (!pending.empty()) pending.erase(pending.begin());
    }
}

// Brings the hub level with the account: the manifest says what the account
// has (names and CRCs) and what it deleted (names and the revision they went
// at). Something on the hub that the account lacks is sent up, unless the
// account deleted it after the hub last synced, in which case it goes.
bool poll() {
    Reply r;
    if (!request(HTTP_METHOD_GET, "/api/hubs/sync", nullptr, nullptr, nullptr, r)) return false;
    if (r.status == 401) {
        // Unlinked on the site: the token is dead, so the link is over here too.
        token.clear();
        email.clear();
        appliedRev = appliedThemeRev = appliedAlertsRev = 0;
        everSynced = false;
        saveLink();
        saveRevs();
        {
            Guard guard;
            pending.clear();
        }
        lastError = "unlinked from the site";
        Serial.println("cloud: the site has unlinked this hub");
        return false;
    }
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    JsonDocument m;
    if (deserializeJson(m, r.body)) {
        lastError = "bad manifest";
        return false;
    }
    const uint32_t rev = m["rev"] | 0;
    bool allOk = true;

    // Configs the account has.
    JsonObjectConst configs = m["configs"].as<JsonObjectConst>();
    for (JsonPairConst kv : configs) {
        const char* name = kv.key().c_str();
        const char* crc = kv.value()["crc"] | "";
        char local[12] = "";
        uint32_t localCrc;
        size_t localSize;
        if (library_store::info(name, localCrc, localSize)) snprintf(local, sizeof(local), "%08lX", (unsigned long)localCrc);
        if (strcmp(local, crc) == 0) continue;
        if (!downloadConfig(name)) allOk = false;
    }
    // Configs the hub has that the account does not.
    std::vector<std::string> names;
    library_store::names(names);
    JsonObjectConst gone = m["deleted"]["configs"].as<JsonObjectConst>();
    for (const std::string& n : names) {
        if (!configs[n.c_str()].isNull()) continue;
        const uint32_t deletedAt = gone[n.c_str()] | 0;
        if (deletedAt > appliedRev && everSynced) {
            library_store::remove(n.c_str());
            Serial.printf("cloud: config %s removed, as on the site\n", n.c_str());
        } else if (!uploadConfig(n.c_str())) {
            allOk = false;
        }
    }

    // Images, the same way.
    JsonObjectConst images = m["images"].as<JsonObjectConst>();
    for (JsonPairConst kv : images) {
        const char* name = kv.key().c_str();
        const char* crc = kv.value()["crc"] | "";
        image_store::Info info;
        char local[12] = "";
        if (image_store::lookup(name, info)) snprintf(local, sizeof(local), "%08lX", (unsigned long)info.crc);
        if (strcmp(local, crc) == 0) continue;
        if (!downloadImage(name)) allOk = false;
    }
    JsonDocument mine;
    image_store::listJson(mine.to<JsonArray>());
    JsonObjectConst goneImages = m["deleted"]["images"].as<JsonObjectConst>();
    for (JsonObjectConst im : mine.as<JsonArrayConst>()) {
        const char* n = im["name"];
        if (!images[n].isNull()) continue;
        const uint32_t deletedAt = goneImages[n] | 0;
        if (deletedAt > appliedRev && everSynced) {
            image_store::remove(n);
            Serial.printf("cloud: image %s removed, as on the site\n", n);
        } else if (!uploadImage(n)) {
            allOk = false;
        }
    }

    // Theme colours and alerts: whichever side changed last.
    const uint32_t themeRev = m["settingsRev"]["themeColours"] | 0;
    const uint32_t alertsRev = m["settingsRev"]["alerts"] | 0;
    if (themeRev > appliedThemeRev || !everSynced) {
        if (!downloadSettings(Kind::ThemeColours, themeRev)) allOk = false;
    }
    if (alertsRev > appliedAlertsRev || !everSynced) {
        if (!downloadSettings(Kind::Alerts, alertsRev)) allOk = false;
    }

    if (allOk) {
        appliedRev = rev;
        everSynced = true;
        saveRevs();
        lastSyncMs = millis();
    }
    return allOk;
}

bool postStatus() {
    JsonDocument doc;
    doc["fw"] = HUB_FW_VERSION;
    doc["board"] = hub::boardName();
    doc["uptime_s"] = millis() / 1000;
    doc["gauges_online"] = hub::stats().gaugesOnline;
    doc["ip"] = WiFi.localIP().toString();
    doc["heap_free"] = ESP.getFreeHeap();
    std::string body;
    serializeJson(doc, body);
    Reply r;
    if (!request(HTTP_METHOD_POST, "/api/hubs/status", &body, nullptr, "application/json", r)) return false;
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    return true;
}

bool link(const std::string& code) {
    JsonDocument doc;
    doc["code"] = code;
    doc["hubId"] = hubId;
    doc["name"] = hubName;
    doc["fw"] = HUB_FW_VERSION;
    doc["board"] = hub::boardName();
    std::string body;
    serializeJson(doc, body);
    token.clear();  // the claim carries no bearer
    Reply r;
    if (!request(HTTP_METHOD_POST, "/api/hubs/claim", &body, nullptr, "application/json", r)) return false;
    if (!r.ok()) {
        lastError = httpError(r);
        return false;
    }
    JsonDocument reply;
    if (deserializeJson(reply, r.body) || !reply["token"].is<const char*>()) {
        lastError = "the site's reply made no sense";
        return false;
    }
    token = reply["token"].as<const char*>();
    email = reply["email"] | "";
    appliedRev = appliedThemeRev = appliedAlertsRev = 0;
    everSynced = false;
    saveLink();
    saveRevs();
    Serial.printf("cloud: linked to %s\n", email.c_str());
    return true;
}

void unlink() {
    if (!token.empty()) {
        Reply r;
        request(HTTP_METHOD_DELETE, (std::string("/api/hubs/") + hubId).c_str(), nullptr, nullptr, nullptr, r);  // best effort
    }
    token.clear();
    email.clear();
    appliedRev = appliedThemeRev = appliedAlertsRev = 0;
    everSynced = false;
    saveLink();
    saveRevs();
    Guard guard;
    pending.clear();
    Serial.println("cloud: unlinked");
}

void cloudTask(void*) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        const uint32_t now = millis();

        std::string code;
        bool wantUnlink = false, wantSync = false, havePending = false;
        {
            Guard guard;
            code.swap(linkCode);
            wantUnlink = unlinkWanted;
            unlinkWanted = false;
            wantSync = syncWanted;
            havePending = !pending.empty();
        }

        if (wantUnlink) {
            unlink();
            setState("not linked", "");
            continue;
        }
        if (token.empty() && state != "not linked" && state != "linking") setState("not linked");
        const bool online = WiFi.status() == WL_CONNECTED;
        if (!code.empty()) {
            if (!online) {
                setState("not linked", "join the home network first");
                continue;
            }
            setState("linking");
            if (link(code)) {
                setState("idle", "");
                wantSync = true;
            } else {
                setState("not linked");
                continue;
            }
        }
        if (token.empty()) continue;
        if (!online) {
            setState("offline");
            continue;
        }
        const bool due = wantSync || int32_t(now - nextPollMs) >= 0;
        const bool statusDue = int32_t(now - nextStatusMs) >= 0;
        if (!due && !havePending && !statusDue) continue;
        if (ESP.getFreeHeap() < kMinHeap) {
            setState("waiting for memory");
            continue;
        }

        setState("syncing");
        bool ok = pushPending();
        if (ok && (due || havePending)) ok = poll();
        if (ok && statusDue) {
            ok = postStatus();
            if (ok) nextStatusMs = now + kStatusMs;
        }
        {
            Guard guard;
            syncWanted = false;
        }
        if (ok) {
            nextPollMs = now + kPollMs;
            setState("idle", "");
        } else {
            nextPollMs = now + kRetryMs;
            setState("error");
            Serial.printf("cloud: %s\n", lastError.c_str());
        }
    }
}

}  // namespace

void begin() {
    mtx = xSemaphoreCreateMutex();
    // The chip's base MAC from efuse: every ESP32 has one, including the P4,
    // whose Wi-Fi MAC belongs to its companion chip.
    uint8_t mac[6] = {};
    esp_efuse_mac_get_default(mac);
    char id[16];
    snprintf(id, sizeof(id), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    hubId = id;
    loadPrefs();
    everSynced = appliedRev != 0;
    state = token.empty() ? "not linked" : "idle";
    nextStatusMs = millis() + 15000;  // soon after boot, then every five minutes
    xTaskCreate(cloudTask, "cloud", 12288, nullptr, 2, &task);
}

bool linked() {
    Guard guard;
    return !token.empty();
}

void requestLink(const char* code) {
    Guard guard;
    linkCode = code ? code : "";
    lastError.clear();
}

void requestUnlink() {
    Guard guard;
    unlinkWanted = true;
}

void requestSync() {
    Guard guard;
    syncWanted = true;
}

void noteLocalChange(Kind kind, const char* name) {
    Guard guard;
    if (token.empty()) return;
    for (Change& c : pending) {
        if (c.kind == kind && c.name == (name ? name : "")) {
            c.deleted = false;
            return;
        }
    }
    pending.push_back({kind, name ? name : "", false});
    syncWanted = true;
}

void noteLocalDelete(Kind kind, const char* name) {
    Guard guard;
    if (token.empty()) return;
    for (Change& c : pending) {
        if (c.kind == kind && c.name == (name ? name : "")) {
            c.deleted = true;
            return;
        }
    }
    pending.push_back({kind, name ? name : "", true});
    syncWanted = true;
}

void statusJson(JsonObject out) {
    Guard guard;
    out["url"] = HUB_CLOUD_URL;
    out["hub_id"] = hubId;
    out["name"] = hubName;
    out["linked"] = !token.empty();
    out["email"] = email;
    out["state"] = state;
    out["error"] = lastError;
    out["last_sync_s"] = lastSyncMs ? int((millis() - lastSyncMs) / 1000) : -1;
    out["rev"] = appliedRev;
    out["pending"] = pending.size();
}

}  // namespace cloud_sync
