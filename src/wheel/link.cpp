#include "link.h"

#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>
#include <Preferences.h>
#include <esp_mac.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#include <string>

#include "board_pins.h"
#include "buttons.h"
#include "gauge_node.h"
#include "proto.h"
#include "version.h"
#include "wifi_fetch.h"

namespace proto = hg::proto;

namespace wheel_link {

namespace {

const char* kPrefs = "wheel";
const char* kNodeKey = "node";
const size_t kMaxPayload = 2048;  // a calibration or a WifiJoin message

proto::GaugeNode node_;
uint32_t currentNode = 0;
volatile bool hub = false;
volatile uint32_t frames = 0;
volatile uint32_t sent = 0;
volatile uint32_t identifyUntilMs = 0;
volatile uint32_t rawUntilMs = 0;
volatile bool reboot = false;
uint8_t* rxBuffer = nullptr;

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
        case proto::Cmd::Identify:
            identifyUntilMs = millis() + uint32_t(arg) * 1000u;
            return true;
        case proto::Cmd::Reboot:
            reboot = true;
            return true;
        case proto::Cmd::InputRaw:
            rawUntilMs = arg ? millis() + uint32_t(arg) * 1000u : 0;
            return true;
        default:
            return false;  // faces, brightness and the rest mean nothing here
    }
}

void onStatus(proto::Status& s, void*) {
    s.configCrc = buttons::calibrationCrc();
    s.activeFace = 0;
    s.faceCount = 0;
    s.vbusMillivolts = 0;  // the C3 Mini has no supply sense
    s.flags = 0;
}

proto::XferStatus onTransferAccept(proto::XferKind kind, uint32_t size, uint8_t** buffer, void*) {
    if (kind != proto::XferKind::WheelCalibration && kind != proto::XferKind::WifiJoin) return proto::XferStatus::BadKind;
    if (size > kMaxPayload) return proto::XferStatus::TooBig;
    if (!rxBuffer) rxBuffer = static_cast<uint8_t*>(malloc(kMaxPayload + 1));
    if (!rxBuffer) return proto::XferStatus::Busy;
    *buffer = rxBuffer;
    return proto::XferStatus::Ok;
}

bool onTransferComplete(proto::XferKind kind, const uint8_t* data, uint32_t size, void*) {
    if (kind == proto::XferKind::WifiJoin) return wifi_fetch::start(data, size);
    std::string error;
    const bool ok = buttons::setCalibration(reinterpret_cast<const char*>(data), size, error);
    if (ok) {
        Serial.printf("link: calibration from the hub stored, CRC %08lX\n", (unsigned long)buttons::calibrationCrc());
    } else {
        Serial.printf("link: the hub's calibration was refused: %s\n", error.c_str());
    }
    return ok;
}

uint32_t loadNode(const uint8_t mac[6]) {
    Preferences prefs;
    prefs.begin(kPrefs, true);
    const uint32_t saved = prefs.getUInt(kNodeKey, proto::kBroadcastNode);
    prefs.end();
    return saved == proto::kBroadcastNode ? proto::GaugeNode::nodeFromMac(mac) : saved;
}

void saveNode(uint32_t id) {
    Preferences prefs;
    prefs.begin(kPrefs, false);
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

// The reports (docs/steering-wheel.md): at once on a change, every
// kInputHoldPeriodMs while a button is down, every kInputHeartbeatMs when idle.
struct Reporter {
    uint8_t seq = 0;
    uint16_t sentMask = 0;
    bool everSent = false;
    uint32_t lastSendMs = 0;
    uint32_t lastRawMs = 0;

    void poll(uint32_t now) {
        const uint16_t mask = buttons::mask();
        const bool raw = rawUntilMs != 0 && int32_t(rawUntilMs - now) > 0;
        const bool changed = mask != sentMask;
        const uint32_t period = mask ? proto::kInputHoldPeriodMs : proto::kInputHeartbeatMs;
        if (changed || !everSent || uint32_t(now - lastSendMs) >= period) {
            proto::Input in;
            in.seq = seq++;
            in.mask = mask;
            in.flags = uint8_t((!changed && everSent ? proto::kInputFlagHeartbeat : 0) |
                               (buttons::fault() ? proto::kInputFlagFault : 0) |
                               (buttons::calibrated() ? 0 : proto::kInputFlagUncalibrated) |
                               (raw ? proto::kInputFlagRaw : 0));
            in.lines = uint8_t(buttons::lineCount());
            if (node_.sendInput(in)) {
                sentMask = mask;
                lastSendMs = now;
                everSent = true;
                sent = sent + 1;
            }
        }
        if (raw && uint32_t(now - lastRawMs) >= 100) {
            for (uint8_t first = 0; first < buttons::lineCount(); first += 3) {
                proto::InputRaw r;
                r.seq = seq++;
                r.first = first;
                for (int i = 0; i < 3; i++) r.mv[i] = buttons::rawMv(size_t(first) + size_t(i));
                node_.sendInputRaw(r);
            }
            lastRawMs = now;
        }
    }
};

void task(void*) {
    ::CanFrame rx;
    Reporter reporter;
    uint32_t lastServiceMs = 0;
    for (;;) {
        bool got = ESP32Can.readFrame(rx, 1);
        while (got) {
            hg::CanFrame f;
            f.id = rx.identifier;
            f.extended = rx.extd;
            f.len = rx.data_length_code > 8 ? 8 : rx.data_length_code;
            memcpy(f.data, rx.data, f.len);
            frames = frames + 1;
            if (f.extended) node_.onFrame(f, millis());
            got = ESP32Can.readFrame(rx, 0);
        }
        const uint32_t now = millis();
        buttons::sample(now);
        node_.poll(now);
        hub = node_.hubPresent();
        reporter.poll(now);

        if (node_.idConflict()) {
            uint32_t id = esp_random() & 0xFFFFFF;
            if (id == proto::kBroadcastNode) id = 0xFFFFFE;
            saveNode(id);
            node_.setNode(id);
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
    ESP32Can.setRxQueueSize(32);
    ESP32Can.setTxQueueSize(16);
    if (!ESP32Can.begin(TWAI_SPEED_1000KBPS)) Serial.println("CAN start failed");

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    currentNode = loadNode(mac);

    proto::Info info;
    info.fwMajor = FW_VERSION_MAJOR;
    info.fwMinor = FW_VERSION_MINOR;
    info.fwPatch = FW_VERSION_PATCH;
    info.hwRevision = proto::kBoardWheel;

    proto::GaugeNode::Handler handler = {};
    handler.command = onCommand;
    handler.status = onStatus;
    handler.transfer = {onTransferAccept, onTransferComplete, nullptr, nullptr};
    node_.init({canSend, nullptr}, mac, currentNode, info, 0, handler);

    xTaskCreatePinnedToCore(task, "link", 6144, nullptr, 18, nullptr, 0);
}

bool hubPresent() { return hub; }
uint32_t node() { return currentNode; }
uint32_t framesReceived() { return frames; }
bool identifying() { return identifyUntilMs != 0 && int32_t(identifyUntilMs - millis()) > 0; }
void requestReboot() { reboot = true; }
bool rebootRequested() { return reboot; }
uint32_t inputsSent() { return sent; }

}  // namespace wheel_link
