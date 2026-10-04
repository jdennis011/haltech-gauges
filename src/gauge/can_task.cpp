#include "can_task.h"

#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>
#include <Preferences.h>
#include <esp_mac.h>

#include <atomic>

#include "board_pins.h"
#include "face_model.h"
#include "gauge_node.h"
#include "version.h"

using namespace hg;

namespace can_task {

namespace {

ChannelStore store;
proto::GaugeNode node;

std::atomic<uint32_t> frames{0};
std::atomic<bool> hub{false};
std::atomic<uint32_t> currentNode{0};
std::atomic<uint16_t> vbus{0};
std::atomic<uint32_t> identifyUntil{0};
std::atomic<int> brightnessRequest{-1};
std::atomic<bool> reboot{false};

// Written by the CAN task, read by the UI loop.
portMUX_TYPE alertMux = portMUX_INITIALIZER_UNLOCKED;
proto::Alert alertNow;

const char* kPrefsNamespace = "gauge";
const char* kNodeKey = "node";

bool canSend(const hg::CanFrame& frame, void*) {
    ::CanFrame out = {};
    out.identifier = frame.id;
    out.extd = frame.extended;
    out.data_length_code = frame.len;
    memcpy(out.data, frame.data, frame.len);
    return ESP32Can.writeFrame(out, 0);
}

bool onCommand(proto::Cmd cmd, uint8_t arg, void*) {
    switch (cmd) {
        case proto::Cmd::Identify: {
            uint32_t until = millis() + uint32_t(arg) * 1000;
            if (until == 0) until = 1;  // 0 means "not requested"
            identifyUntil = until;
            return true;
        }
        case proto::Cmd::SetBrightness:
            brightnessRequest = arg;
            return true;
        case proto::Cmd::Reboot:
            reboot = true;
            return true;
        default:
            // Faces and stored configs arrive with the face builder.
            return false;
    }
}

void onAlert(const proto::Alert& alert, void*) {
    portENTER_CRITICAL(&alertMux);
    alertNow = alert;
    portEXIT_CRITICAL(&alertMux);
    if (alert.show) {
        Serial.printf("Alert: %s\n", alert.text);
    } else {
        Serial.println("Alert cleared");
    }
}

void onStatus(proto::Status& status, void*) {
    status.configCrc = 0;
    status.activeFace = 0;
    status.faceCount = 0;
    status.vbusMillivolts = vbus;
    status.flags = 0;
}

proto::XferStatus refuseTransfer(proto::XferKind, uint32_t, uint8_t**, void*) {
    return proto::XferStatus::Busy;
}
bool ignoreTransfer(proto::XferKind, const uint8_t*, uint32_t, void*) { return false; }

uint32_t loadNode(const uint8_t mac[6]) {
    Preferences prefs;
    prefs.begin(kPrefsNamespace, true);
    const uint32_t saved = prefs.getUInt(kNodeKey, proto::kBroadcastNode);
    prefs.end();
    return saved == proto::kBroadcastNode ? proto::GaugeNode::nodeFromMac(mac) : saved;
}

void saveNode(uint32_t id) {
    Preferences prefs;
    prefs.begin(kPrefsNamespace, false);
    prefs.putUInt(kNodeKey, id);
    prefs.end();
}

void serviceBus() {
    const uint32_t state = ESP32Can.canState();
    if (state == TWAI_STATE_BUS_OFF) {
        ESP32Can.recover();
    } else if (state == TWAI_STATE_STOPPED) {
        ESP32Can.restart();
    }
}

void task(void*) {
    ::CanFrame rx;
    uint32_t lastServiceMs = 0;
    for (;;) {
        bool got = ESP32Can.readFrame(rx, 5);
        while (got) {
            hg::CanFrame f;
            f.id = rx.identifier;
            f.extended = rx.extd;
            f.len = rx.data_length_code > 8 ? 8 : rx.data_length_code;
            memcpy(f.data, rx.data, f.len);
            frames++;

            const uint32_t now = millis();
            if (f.extended) {
                node.onFrame(f, now);
            } else {
                store.onFrame(f, now);
            }
            got = ESP32Can.readFrame(rx, 0);
        }

        const uint32_t now = millis();
        node.poll(now);
        hub = node.hubPresent();

        if (node.idConflict()) {
            // Another gauge has the same id: take a random one and keep it.
            uint32_t id = esp_random() & 0xFFFFFF;
            if (id == proto::kBroadcastNode) id = 0xFFFFFE;
            saveNode(id);
            node.setNode(id);
            currentNode = id;
        }

        if (now - lastServiceMs >= 500) {
            lastServiceMs = now;
            serviceBus();
        }
    }
}

}  // namespace

void begin() {
    ESP32Can.setPins(PIN_CAN_TX, PIN_CAN_RX);
    ESP32Can.setRxQueueSize(128);
    ESP32Can.setTxQueueSize(16);
    if (!ESP32Can.begin(TWAI_SPEED_1000KBPS)) Serial.println("CAN start failed");

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    const uint32_t id = loadNode(mac);
    currentNode = id;

    proto::Info info;
    info.fwMajor = FW_VERSION_MAJOR;
    info.fwMinor = FW_VERSION_MINOR;
    info.fwPatch = FW_VERSION_PATCH;

    proto::GaugeNode::Handler handler = {};
    handler.command = onCommand;
    handler.status = onStatus;
    handler.alert = onAlert;
    handler.transfer = {refuseTransfer, ignoreTransfer, nullptr, nullptr};
    node.init({canSend, nullptr}, mac, id, info, cfg::kSchemaVersion, handler);

    xTaskCreatePinnedToCore(task, "can", 6144, nullptr, 18, nullptr, 0);
}

ChannelStore& channels() { return store; }
uint32_t framesReceived() { return frames; }
bool hubPresent() { return hub; }
uint32_t nodeId() { return currentNode; }
void setVbusMillivolts(uint16_t millivolts) { vbus = millivolts; }
uint32_t identifyUntilMs() { return identifyUntil; }
int takeBrightnessRequest() { return brightnessRequest.exchange(-1); }
bool rebootRequested() { return reboot; }

bool alertMessage(char* text, uint32_t& color) {
    portENTER_CRITICAL(&alertMux);
    const bool show = alertNow.show;
    if (show) {
        memcpy(text, alertNow.text, sizeof(alertNow.text));
        color = alertNow.color;
    }
    portEXIT_CRITICAL(&alertMux);
    return show;
}

}  // namespace can_task
