#include "wifi_xfer.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdio.h>
#include <string.h>

#include "gauge_firmware.h"
#include "hub_config.h"
#include "hub_state.h"
#include "image_store.h"

using namespace hg;
using proto::HubManager;

namespace wifi_xfer {

namespace {

const char* kHost = "192.168.4.1";  // the access point's address (wifi_ap.cpp)
const uint16_t kPort = 80;
const uint32_t kJoinMs = 20 * 1000;      // joining the access point and the first request
const uint32_t kBytesPerMs = 50;         // 50 KB/s: a slow fetch
const uint32_t kGraceMs = 60 * 1000;     // a token outlives the hub's wait by this much
const uint32_t kFirmwareTokenMs = 5 * 60 * 1000;
const size_t kMaxJobs = 8;

struct Job {
    bool used = false;
    char token[33] = {};
    uint32_t node = 0;
    uint32_t expiresMs = 0;
    uint32_t crcs[HubManager::kMaxImages] = {};
    size_t count = 0;
    bool firmware = false;
    std::string firmwarePath;
};

SemaphoreHandle_t mtx = nullptr;
Job jobTable[kMaxJobs];
// The WifiJoin messages being sent: each stays valid until the manager is
// done with it, and the manager sends one transfer at a time.
std::string imagePayload;
std::string firmwarePayload;

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

bool reached(uint32_t nowMs, uint32_t whenMs) { return int32_t(nowMs - whenMs) >= 0; }

void expire(uint32_t nowMs) {
    for (Job& j : jobTable) {
        if (j.used && reached(nowMs, j.expiresMs)) j.used = false;
    }
}

// A fresh job with a random token; the oldest is dropped if the table is full.
Job* newJob(uint32_t node, uint32_t ttlMs) {
    const uint32_t now = millis();
    expire(now);
    Job* job = nullptr;
    for (Job& j : jobTable) {
        if (!j.used) {
            job = &j;
            break;
        }
    }
    if (!job) {
        job = &jobTable[0];
        for (Job& j : jobTable) {
            if (int32_t(j.expiresMs - job->expiresMs) < 0) job = &j;
        }
    }
    *job = Job();
    job->used = true;
    job->node = node;
    job->expiresMs = now + ttlMs;
    for (int i = 0; i < 4; i++) snprintf(job->token + 8 * i, 9, "%08lx", (unsigned long)esp_random());
    return job;
}

void fillAccess(JsonDocument& doc, const Job& job) {
    doc["ssid"] = HUB_AP_SSID;
    doc["pass"] = HUB_AP_PASSWORD;
    doc["host"] = kHost;
    doc["port"] = kPort;
    doc["token"] = job.token;
}

}  // namespace

void begin() { mtx = xSemaphoreCreateMutex(); }

bool offerImages(const HubManager::Gauge& g, const uint32_t* crcs, size_t count, const uint8_t** data,
                 uint32_t* size, uint32_t* waitMs, void*) {
    if (!count) return false;
    uint32_t total = 0;
    JsonDocument doc;
    Guard guard;
    const uint32_t wait = kJoinMs;  // extended below once the sizes are known
    Job* job = newJob(g.node, wait + kGraceMs);
    JsonArray list = doc["images"].to<JsonArray>();
    for (size_t i = 0; i < count && job->count < HubManager::kMaxImages; i++) {
        image_store::Info info;
        if (!image_store::lookupCrc(crcs[i], info)) continue;  // gone from the hub: the bus path skips it
        char hex[9];
        snprintf(hex, sizeof(hex), "%08lX", (unsigned long)crcs[i]);
        list.add(hex);
        job->crcs[job->count++] = crcs[i];
        total += info.size;
    }
    if (!job->count) {
        job->used = false;
        return false;
    }
    fillAccess(doc, *job);
    imagePayload.clear();
    serializeJson(doc, imagePayload);
    *data = reinterpret_cast<const uint8_t*>(imagePayload.data());
    *size = uint32_t(imagePayload.size());
    *waitMs = kJoinMs + total / kBytesPerMs;
    job->expiresMs = millis() + *waitMs + kGraceMs;
    Serial.printf("wifi: offering %u image(s), %lu bytes, to gauge %06lX\n", unsigned(job->count), (unsigned long)total,
                  (unsigned long)g.node);
    return true;
}

bool sendFirmware(uint32_t node, std::string& error) {
    hub::Lock lock;
    HubManager& m = hub::manager();
    const HubManager::Gauge* g = m.find(node);
    if (!g || !g->online || !g->haveHello) {
        error = "the gauge is offline";
        return false;
    }
    if (!g->haveInfo) {
        error = "the gauge has not said which board it is";
        return false;
    }
    gauge_firmware::Build build;
    if (!gauge_firmware::stored(g->info.hwRevision, build)) {
        error = "the hub has no firmware for this gauge's board: fetch it under Settings, Firmware";
        return false;
    }
    if (m.pushBusy()) {
        error = "a transfer is already running; try again in a moment";
        return false;
    }
    Guard guard;
    Job* job = newJob(node, kFirmwareTokenMs);
    job->firmware = true;
    job->firmwarePath = build.path;
    JsonDocument doc;
    fillAccess(doc, *job);
    JsonObject fw = doc["firmware"].to<JsonObject>();
    fw["size"] = build.size;
    fw["sha256"] = build.sha256;
    fw["version"] = build.version;
    firmwarePayload.clear();
    serializeJson(doc, firmwarePayload);
    if (!m.wifiJoin(node, reinterpret_cast<const uint8_t*>(firmwarePayload.data()), uint32_t(firmwarePayload.size()),
                    millis())) {
        job->used = false;
        error = "could not send: the bus is busy";
        return false;
    }
    Serial.printf("wifi: firmware %s offered to gauge %06lX\n", build.version.c_str(), (unsigned long)node);
    return true;
}

bool allowsImage(const char* token, uint32_t crc) {
    if (!token || strlen(token) != 32) return false;
    Guard guard;
    expire(millis());
    for (const Job& j : jobTable) {
        if (!j.used || j.firmware || strcmp(j.token, token) != 0) continue;
        for (size_t i = 0; i < j.count; i++) {
            if (j.crcs[i] == crc) return true;
        }
    }
    return false;
}

bool allowsFirmware(const char* token, std::string& path) {
    if (!token || strlen(token) != 32) return false;
    Guard guard;
    expire(millis());
    for (const Job& j : jobTable) {
        if (j.used && j.firmware && strcmp(j.token, token) == 0) {
            path = j.firmwarePath;
            return true;
        }
    }
    return false;
}

size_t jobs() {
    Guard guard;
    expire(millis());
    size_t n = 0;
    for (const Job& j : jobTable) {
        if (j.used) n++;
    }
    return n;
}

}  // namespace wifi_xfer
