#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_system.h>

#include <atomic>

#include "can_ports.h"
#include "channel_store.h"
#include "display_control.h"
#include "haltech.h"
#include "hub_config.h"
#include "hub_manager.h"
#include "hub_state.h"
#include "library_store.h"
#include "simulator.h"
#include "virtual_gauges.h"
#include "web_server.h"
#include "wifi_ap.h"

using namespace hg;
using proto::HubManager;

// The web UI's JSON is built on the loop task.
SET_LOOP_TASK_STACK_SIZE(12 * 1024);

namespace {

#if HG_HAS_ECU_PORT
EcuPort ecuPort;
#endif
GaugePort gaugePort;
HubManager hubManager;
ChannelStore liveStore;
SemaphoreHandle_t hubLock = nullptr;

// Management frames may occupy at most this many transmit slots, so live data
// always finds room.
const uint32_t kManagementTxLimit = 8;

std::atomic<bool> gaugesOnline{false};
std::atomic<uint32_t> onlineCount{0};
std::atomic<uint32_t> ecuFrames{0};
std::atomic<uint32_t> forwarded{0};
std::atomic<uint32_t> forwardDropped{0};
std::atomic<uint32_t> rosterChanges{0};

// The CAN library also defines a global CanFrame, so the shared type is always
// written out in full in this file.
bool managementSend(const hg::CanFrame& frame, void*) {
    if (gaugePort.txPending() >= kManagementTxLimit) return false;
    return gaugePort.write(frame);
}

// Gauge-to-hub config uploads arrive with a later phase.
proto::XferStatus refuseUpload(proto::XferKind, uint32_t, uint8_t**, void*) {
    return proto::XferStatus::Busy;
}
bool ignoreUpload(proto::XferKind, const uint8_t*, uint32_t, void*) { return false; }

void onRosterChanged(void*) { rosterChanges++; }

#if HG_HAS_ECU_PORT
// Forwards Haltech broadcast frames from the ECU bus to the gauge bus. With
// no gauge listening nothing would acknowledge them, so they are only sent
// while the roster is non-empty.
void bridgeTask(void*) {
    hg::CanFrame frame;
    for (;;) {
        if (!ecuPort.read(frame, 100)) continue;
        ecuFrames++;
        if (frame.extended || !findFrame(frame.id)) continue;
        if (simulator::enabled()) continue;  // the simulator owns the live data and the bus
        liveStore.onFrame(frame, millis());
        if (!gaugesOnline) continue;
        if (gaugePort.write(frame)) {
            forwarded++;
        } else {
            forwardDropped++;
        }
    }
}
#endif

// Everything that talks on the gauge bus other than forwarding: beacon,
// roster, transfers and the simulator.
void gaugeBusTask(void*) {
    hg::CanFrame frame;
    for (;;) {
        // Waits up to one tick for a frame, which also paces the loop at about 1 kHz.
        const bool got = gaugePort.read(frame, 1);
        hub::Lock lock;
        const uint32_t now = millis();
        if (got) {
            hubManager.onFrame(frame, now);
            while (gaugePort.read(frame, 0)) hubManager.onFrame(frame, now);
        }
        hubManager.poll(now);

        uint32_t online = 0;
        for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
            const HubManager::Gauge& g = hubManager.gauge(i);
            if (g.used && g.online) online++;
        }
        onlineCount = online;
        gaugesOnline = online > 0;
        simulator::poll(now, liveStore, gaugesOnline ? &gaugePort : nullptr);
    }
}

void printRoster() {
    hub::Lock lock;
    Serial.println("Gauges:");
    bool any = false;
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = hubManager.gauge(i);
        if (!g.used) continue;
        any = true;
        Serial.printf("  node %06lX  %s", (unsigned long)g.node, g.online ? "online " : "offline");
        if (g.haveHello) {
            Serial.printf("  mac %02X:%02X:%02X:%02X:%02X:%02X", g.mac[0], g.mac[1], g.mac[2],
                          g.mac[3], g.mac[4], g.mac[5]);
        }
        if (g.haveInfo) Serial.printf("  fw %u.%u.%u", g.info.fwMajor, g.info.fwMinor, g.info.fwPatch);
        if (g.haveStatus) {
            Serial.printf("  face %u/%u  config %08lX  vbus %u mV", g.status.activeFace,
                          g.status.faceCount, (unsigned long)g.status.configCrc,
                          g.status.vbusMillivolts);
        }
        if (g.duplicateId) Serial.print("  DUPLICATE ID");
        Serial.println();
    }
    if (!any) Serial.println("  none");
}

void handleConsole() {
    while (Serial.available()) {
        switch (Serial.read()) {
            case 's':
                hub::setSimulator(!hub::simulatorEnabled());
                Serial.printf("Simulator %s\n", hub::simulatorEnabled() ? "on" : "off");
                break;
            case 'r':
                printRoster();
                break;
            case 'i': {
                hub::Lock lock;
                for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
                    const HubManager::Gauge& g = hubManager.gauge(i);
                    if (g.used && g.online) hubManager.command(g.node, proto::Cmd::Identify, 5);
                }
                Serial.println("Identify sent");
                break;
            }
            case 'w': {
                JsonDocument doc;
                wifi_ap::statusJson(doc.to<JsonObject>());
                serializeJson(doc, Serial);
                Serial.println();
                break;
            }
            case 'W': {
                // W<network>;<password> joins a home network; W on its own forgets it.
                Serial.setTimeout(5000);
                String line = Serial.readStringUntil('\n');
                line.trim();
                const int sep = line.indexOf(';');
                const String ssid = sep < 0 ? line : line.substring(0, sep);
                const String password = sep < 0 ? String() : line.substring(sep + 1);
                wifi_ap::setStation(ssid.c_str(), password.c_str());
                Serial.println(ssid.isEmpty() ? "Home network forgotten" : "Home network saved, connecting");
                break;
            }
            case '?':
            case 'h':
                Serial.println("s = toggle simulator, r = list gauges, i = identify all, w = wifi status,");
                Serial.println("W<network>;<password> = join a home network (W alone forgets it)");
                break;
            default:
                break;
        }
    }
}

}  // namespace

namespace hub {

SemaphoreHandle_t lock() { return hubLock; }
HubManager& manager() { return hubManager; }
ChannelStore& live() { return liveStore; }

HubStats stats() {
    HubStats s = {};
#if HG_HAS_ECU_PORT
    s.ecuPresent = true;
    s.ecuFrames = ecuFrames;
    s.ecu = ecuPort.stats();
#endif
    s.forwarded = forwarded;
    s.forwardDropped = forwardDropped;
    s.simulated = simulator::framesGenerated();
    s.bus = gaugePort.stats();
    s.txPending = gaugePort.txPending();
    s.simulator = simulator::enabled();
    s.gaugesOnline = onlineCount;
    return s;
}

void setSimulator(bool on) {
    Lock lock;
    simulator::setEnabled(on);
    hubManager.setBeaconFlags(on ? proto::kBeaconFlagSimulator : 0);
}

bool simulatorEnabled() { return simulator::enabled(); }

uint32_t rosterVersion() { return rosterChanges; }
void rosterTouched() { rosterChanges++; }

const char* boardName() {
#if CONFIG_IDF_TARGET_ESP32C6
    return "ESP32-C6-DevKitC-1";
#elif defined(ARDUINO_LOLIN_C3_MINI)
    return "Lolin C3 Mini (development: no ECU bridge)";
#else
    return "ESP32-C3 (development: no ECU bridge)";
#endif
}

const char* resetReason() {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON: return "power on";
        case ESP_RST_EXT: return "reset pin";
        case ESP_RST_SW: return "software restart";
        case ESP_RST_PANIC: return "panic";
        case ESP_RST_INT_WDT: return "interrupt watchdog";
        case ESP_RST_TASK_WDT: return "task watchdog";
        case ESP_RST_WDT: return "watchdog";
        case ESP_RST_DEEPSLEEP: return "deep sleep";
        case ESP_RST_BROWNOUT: return "brownout";
        case ESP_RST_SDIO: return "sdio";
        case ESP_RST_USB: return "usb";
        case ESP_RST_JTAG: return "jtag";
        default: return "unknown";
    }
}

}  // namespace hub

void setup() {
    Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
    Serial.setTxTimeoutMs(0);  // never block when no USB host is attached
#endif
    hubLock = xSemaphoreCreateMutex();

#if HG_HAS_ECU_PORT
    if (!ecuPort.begin()) Serial.println("ECU CAN start failed");
#else
    Serial.println("Single-controller build: gauge bus only, no ECU bridge");
#endif
    if (!gaugePort.begin()) Serial.println("Gauge CAN start failed");
    if (!library_store::begin()) Serial.println("Config storage failed to mount");
    virtual_gauges::begin();
    display_control::begin();

    HubManager::Handler handler = {};
    handler.desired = library_store::desired;
    handler.upload = {refuseUpload, ignoreUpload, nullptr, nullptr};
    handler.changed = onRosterChanged;
    handler.online = display_control::onOnline;
    handler.request = display_control::onRequest;
    hubManager.init({managementSend, nullptr}, handler, millis());

    wifi_ap::begin();
    web_server::begin();

#if HG_HAS_ECU_PORT
    xTaskCreate(bridgeTask, "bridge", 4096, nullptr, 20, nullptr);
#endif
    xTaskCreate(gaugeBusTask, "gaugebus", 6144, nullptr, 15, nullptr);

    Serial.printf("Hub %s on %s\nWi-Fi \"%s\" password %s, then http://192.168.4.1/\n"
                  "Press ? for console commands.\n",
                  HUB_FW_VERSION, hub::boardName(), HUB_AP_SSID, HUB_AP_PASSWORD);
}

void loop() {
#if HG_HAS_ECU_PORT
    ecuPort.service();
#endif
    gaugePort.service();
    handleConsole();
    wifi_ap::loop();
    web_server::loop();
    {
        hub::Lock lock;
        if (!hubManager.pushBusy()) library_store::collectGarbage();
    }

    static uint32_t printedRoster = 0;
    if (rosterChanges != printedRoster) {
        printedRoster = rosterChanges;
        printRoster();
    }

    static uint32_t lastPrint = 0;
    if (millis() - lastPrint >= 10000) {
        lastPrint = millis();
        const HubStats s = hub::stats();
#if HG_HAS_ECU_PORT
        Serial.printf("ecu: %lu frames, missed %lu, state %lu | ", (unsigned long)s.ecuFrames,
                      (unsigned long)s.ecu.rxMissed, (unsigned long)s.ecu.state);
#endif
        Serial.printf("gauge bus: online %lu, forwarded %lu, dropped %lu, simulated %lu, missed %lu, "
                      "state %lu | heap %u (min %u)\n",
                      (unsigned long)s.gaugesOnline, (unsigned long)s.forwarded,
                      (unsigned long)s.forwardDropped, (unsigned long)s.simulated,
                      (unsigned long)s.bus.rxMissed, (unsigned long)s.bus.state,
                      unsigned(ESP.getFreeHeap()), unsigned(ESP.getMinFreeHeap()));
    }
    delay(5);
}
