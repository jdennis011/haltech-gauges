#include "wifi_fetch.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <esp_http_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "can_task.h"
#include "crc32.h"
#include "image_store.h"
#include "ip_link.h"
#include "proto.h"

namespace wifi_fetch {

namespace {

const uint32_t kJoinTimeoutMs = 15 * 1000;
const uint32_t kRequestTimeoutMs = 30 * 1000;
const size_t kChunk = 4096;

struct Job {
    std::string ssid, pass, host, token;
    uint16_t port = 80;
    std::vector<uint32_t> images;
    bool firmware = false;
    uint32_t firmwareSize = 0;
    std::string firmwareSha;
    std::string firmwareVersion;
};

SemaphoreHandle_t mtx = nullptr;
TaskHandle_t task = nullptr;
Job pending;        // under mtx, handed to the task
bool havePending = false;
bool running = false;
const char* stateText = "idle";

struct Guard {
    Guard() { xSemaphoreTake(mtx, portMAX_DELAY); }
    ~Guard() { xSemaphoreGive(mtx); }
};

std::string url(const Job& j, const char* path) {
    char buf[160];
    snprintf(buf, sizeof(buf), "http://%s:%u%s?token=%s", j.host.c_str(), unsigned(j.port), path, j.token.c_str());
    return buf;
}

typedef bool (*Sink)(const uint8_t* data, size_t len, void* ctx);

// GET `u`, handing the body to `sink`. Returns the HTTP status, or -1 with why.
int httpGet(const std::string& u, Sink sink, void* ctx, std::string& why, int64_t& length) {
    length = -1;
    esp_http_client_config_t cfg = {};
    cfg.url = u.c_str();
    cfg.method = HTTP_METHOD_GET;
    cfg.timeout_ms = kRequestTimeoutMs;
    cfg.buffer_size = 4096;
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) {
        why = "could not start a request";
        return -1;
    }
    if (esp_http_client_open(c, 0) != ESP_OK) {
        why = "could not reach the hub";
        esp_http_client_cleanup(c);
        return -1;
    }
    length = esp_http_client_fetch_headers(c);
    if (length < 0) {
        why = "no reply from the hub";
        esp_http_client_close(c);
        esp_http_client_cleanup(c);
        return -1;
    }
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

struct ImageDownload {
    uint8_t* data = nullptr;
    uint32_t capacity = 0;
    uint32_t size = 0;
};

bool toImage(const uint8_t* data, size_t len, void* ctx) {
    ImageDownload* d = static_cast<ImageDownload*>(ctx);
    if (!d->data) {
        // The size is known from the headers; the whole image goes to PSRAM.
        d->data = static_cast<uint8_t*>(heap_caps_malloc(d->capacity ? d->capacity : 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        if (!d->data) d->data = static_cast<uint8_t*>(malloc(d->capacity ? d->capacity : 1));
        if (!d->data) return false;
    }
    if (d->size + len > d->capacity) return false;
    memcpy(d->data + d->size, data, len);
    d->size += uint32_t(len);
    return true;
}

bool fetchImage(const Job& j, uint32_t crc) {
    char path[40];
    snprintf(path, sizeof(path), "/gauge/image/%08lX", (unsigned long)crc);
    // Length first, so the buffer is right-sized: a HEAD is not worth a
    // round trip; the GET's headers give it before the body.
    ImageDownload d;
    std::string why;
    int64_t length;
    const std::string u = url(j, path);
    // One pass: capacity comes from Content-Length once known; the first sink
    // call happens after the headers, so set it from a probe of the size.
    esp_http_client_config_t cfg = {};
    cfg.url = u.c_str();
    cfg.method = HTTP_METHOD_HEAD;
    cfg.timeout_ms = kRequestTimeoutMs;
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (c && esp_http_client_open(c, 0) == ESP_OK) {
        const int64_t len = esp_http_client_fetch_headers(c);
        if (esp_http_client_get_status_code(c) == 200 && len > 0 && len <= int64_t(image_store::maxImageBytes())) {
            d.capacity = uint32_t(len);
        }
        esp_http_client_close(c);
    }
    if (c) esp_http_client_cleanup(c);
    if (!d.capacity) {
        Serial.printf("wifi: image %08lX: the hub did not say its size\n", (unsigned long)crc);
        return false;
    }
    if (!image_store::roomFor(d.capacity)) {
        Serial.printf("wifi: image %08lX: no room\n", (unsigned long)crc);
        return false;
    }
    const int status = httpGet(u, toImage, &d, why, length);
    if (status != 200 || d.size != d.capacity) {
        Serial.printf("wifi: image %08lX: %s\n", (unsigned long)crc, status < 0 ? why.c_str() : "bad reply");
        free(d.data);
        return false;
    }
    if (hg::crc32(d.data, d.size) != crc) {
        Serial.printf("wifi: image %08lX: the bytes do not match their name\n", (unsigned long)crc);
        free(d.data);
        return false;
    }
    if (!image_store::submit(d.data, d.size)) return false;  // the store owns or freed the buffer
    can_task::reportImage(crc);
    Serial.printf("wifi: image %08lX, %lu bytes, stored\n", (unsigned long)crc, (unsigned long)d.size);
    return true;
}

struct FirmwareDownload {
    mbedtls_sha256_context sha;
    uint32_t written = 0;
    uint32_t expected = 0;
    std::string why;
};

bool toSlot(const uint8_t* data, size_t len, void* ctx) {
    FirmwareDownload* d = static_cast<FirmwareDownload*>(ctx);
    if (d->written + len > d->expected) {
        d->why = "the file is larger than announced";
        return false;
    }
    if (Update.write(const_cast<uint8_t*>(data), len) != len) {
        d->why = Update.errorString();
        return false;
    }
    mbedtls_sha256_update(&d->sha, data, len);
    d->written += len;
    return true;
}

bool fetchFirmware(const Job& j) {
    if (!Update.begin(j.firmwareSize)) {
        Serial.printf("wifi: firmware: %s\n", Update.errorString());
        return false;
    }
    FirmwareDownload d;
    d.expected = j.firmwareSize;
    mbedtls_sha256_init(&d.sha);
    mbedtls_sha256_starts(&d.sha, 0);
    int64_t length;
    const int status = httpGet(url(j, "/gauge/firmware"), toSlot, &d, d.why, length);
    uint8_t digest[32];
    mbedtls_sha256_finish(&d.sha, digest);
    mbedtls_sha256_free(&d.sha);
    std::string why;
    if (status < 0) {
        why = d.why;
    } else if (status != 200) {
        why = "the hub refused the download";
    } else if (d.written != j.firmwareSize) {
        why = "the download was cut short";
    } else {
        char hex[65];
        for (int k = 0; k < 32; k++) snprintf(hex + 2 * k, 3, "%02x", digest[k]);
        if (j.firmwareSha != hex) why = "the file's SHA-256 does not match";
    }
    if (!why.empty()) {
        Update.abort();
        Serial.printf("wifi: firmware: %s\n", why.c_str());
        return false;
    }
    if (!Update.end()) {
        Serial.printf("wifi: firmware: %s\n", Update.errorString());
        return false;
    }
    Serial.printf("wifi: firmware %s installed; restarting\n", j.firmwareVersion.c_str());
    return true;
}

void run(const Job& j) {
    stateText = "joining";
    // A gauge linked to the hub over Wi-Fi may be on that network already.
    const bool already = WiFi.status() == WL_CONNECTED && WiFi.SSID() == j.ssid.c_str();
    if (!already) {
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(j.ssid.c_str(), j.pass.c_str());
    }
    const uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started < kJoinTimeoutMs) vTaskDelay(pdMS_TO_TICKS(100));
    bool restart = false;
    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("wifi: could not join %s\n", j.ssid.c_str());
    } else {
        Serial.printf("wifi: joined %s as %s\n", j.ssid.c_str(), WiFi.localIP().toString().c_str());
        stateText = "fetching";
        for (uint32_t crc : j.images) {
            if (image_store::has(crc)) {
                can_task::reportImage(crc);
                continue;
            }
            fetchImage(j, crc);
        }
        if (j.firmware) restart = fetchFirmware(j);
    }
    stateText = "leaving";
    if (!ip_link::enabled()) {
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
    }
    stateText = "idle";
    if (restart) {
        vTaskDelay(pdMS_TO_TICKS(500));  // let the last status and acks go out
        can_task::requestReboot();
    }
}

void wifiTask(void*) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Job job;
        {
            Guard guard;
            if (!havePending) continue;
            job = pending;
            havePending = false;
            running = true;
        }
        run(job);
        Guard guard;
        running = false;
    }
}

bool parse(const uint8_t* data, size_t size, Job& out) {
    JsonDocument doc;
    if (deserializeJson(doc, data, size) != DeserializationError::Ok || !doc.is<JsonObject>()) return false;
    JsonObjectConst o = doc.as<JsonObjectConst>();
    out.ssid = o["ssid"] | "";
    out.pass = o["pass"] | "";
    out.host = o["host"] | "";
    out.port = uint16_t(o["port"] | 80);
    out.token = o["token"] | "";
    if (out.ssid.empty() || out.host.empty() || out.token.empty()) return false;
    for (JsonVariantConst v : o["images"].as<JsonArrayConst>()) {
        const char* hex = v.as<const char*>();
        if (hex && strlen(hex) == 8) out.images.push_back(uint32_t(strtoul(hex, nullptr, 16)));
    }
    JsonObjectConst fw = o["firmware"].as<JsonObjectConst>();
    if (!fw.isNull()) {
        out.firmware = true;
        out.firmwareSize = fw["size"] | 0;
        out.firmwareSha = fw["sha256"] | "";
        out.firmwareVersion = fw["version"] | "";
        for (char& c : out.firmwareSha) c = char(tolower(uint8_t(c)));
        if (!out.firmwareSize || out.firmwareSha.size() != 64) return false;
    }
    return !out.images.empty() || out.firmware;
}

}  // namespace

void begin() {
    mtx = xSemaphoreCreateMutex();
    WiFi.mode(WIFI_OFF);
    xTaskCreatePinnedToCore(wifiTask, "wifi", 10240, nullptr, 3, &task, 0);
}

bool start(const uint8_t* data, size_t size) {
    Job job;
    if (!parse(data, size, job)) {
        Serial.println("wifi: the hub's join message was not understood");
        return false;
    }
    {
        Guard guard;
        if (running || havePending) return false;
        pending = job;
        havePending = true;
    }
    xTaskNotifyGive(task);
    return true;
}

bool busy() {
    Guard guard;
    return running || havePending;
}

const char* state() { return stateText; }

}  // namespace wifi_fetch
