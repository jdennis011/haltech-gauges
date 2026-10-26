#include "wifi_fetch.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_http_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "link.h"

namespace wifi_fetch {

namespace {

const uint32_t kJoinTimeoutMs = 15 * 1000;
const uint32_t kRequestTimeoutMs = 30 * 1000;
const size_t kChunk = 4096;

struct Job {
    std::string ssid, pass, host, token;
    uint16_t port = 80;
    uint32_t firmwareSize = 0;
    std::string firmwareSha;
    std::string firmwareVersion;
};

SemaphoreHandle_t mtx = nullptr;
TaskHandle_t task = nullptr;
Job pending;
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

struct Download {
    mbedtls_sha256_context sha;
    uint32_t written = 0;
    uint32_t expected = 0;
    std::string why;
};

bool toSlot(Download& d, const uint8_t* data, size_t len) {
    if (d.written + len > d.expected) {
        d.why = "the file is larger than announced";
        return false;
    }
    if (Update.write(const_cast<uint8_t*>(data), len) != len) {
        d.why = Update.errorString();
        return false;
    }
    mbedtls_sha256_update(&d.sha, data, len);
    d.written += uint32_t(len);
    return true;
}

// GET the image into the spare slot. Returns the HTTP status, or -1 with why.
int fetch(const Job& j, Download& d) {
    const std::string u = url(j, "/gauge/firmware");
    esp_http_client_config_t cfg = {};
    cfg.url = u.c_str();
    cfg.method = HTTP_METHOD_GET;
    cfg.timeout_ms = kRequestTimeoutMs;
    cfg.buffer_size = 4096;
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) {
        d.why = "could not start a request";
        return -1;
    }
    if (esp_http_client_open(c, 0) != ESP_OK) {
        d.why = "could not reach the hub";
        esp_http_client_cleanup(c);
        return -1;
    }
    if (esp_http_client_fetch_headers(c) < 0) {
        d.why = "no reply from the hub";
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
            d.why = "the download broke off";
            ok = false;
            break;
        }
        if (n == 0) break;
        if (status >= 200 && status < 300) ok = toSlot(d, buf.data(), size_t(n));
    }
    esp_http_client_close(c);
    esp_http_client_cleanup(c);
    return ok ? status : -1;
}

bool fetchFirmware(const Job& j) {
    if (!Update.begin(j.firmwareSize)) {
        Serial.printf("wifi: firmware: %s\n", Update.errorString());
        return false;
    }
    Download d;
    d.expected = j.firmwareSize;
    mbedtls_sha256_init(&d.sha);
    mbedtls_sha256_starts(&d.sha, 0);
    const int status = fetch(j, d);
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
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(j.ssid.c_str(), j.pass.c_str());
    const uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started < kJoinTimeoutMs) vTaskDelay(pdMS_TO_TICKS(100));
    bool restart = false;
    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("wifi: could not join %s\n", j.ssid.c_str());
    } else {
        Serial.printf("wifi: joined %s as %s\n", j.ssid.c_str(), WiFi.localIP().toString().c_str());
        stateText = "fetching";
        restart = fetchFirmware(j);
    }
    stateText = "leaving";
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    stateText = "idle";
    if (restart) {
        vTaskDelay(pdMS_TO_TICKS(500));  // let the last status and acks go out
        wheel_link::requestReboot();
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
    JsonObjectConst fw = o["firmware"].as<JsonObjectConst>();
    if (fw.isNull()) return false;  // images mean nothing to a wheel
    out.firmwareSize = fw["size"] | 0;
    out.firmwareSha = fw["sha256"] | "";
    out.firmwareVersion = fw["version"] | "";
    for (char& c : out.firmwareSha) c = char(tolower(uint8_t(c)));
    return out.firmwareSize && out.firmwareSha.size() == 64;
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
