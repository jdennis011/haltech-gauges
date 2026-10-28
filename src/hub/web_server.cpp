#include "web_server.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <string>
#include <vector>

#include "alert_monitor.h"
#include "channel_store.h"
#include "cloud_sync.h"
#include "datalog.h"
#include "display_control.h"
#include "face_model.h"
#include "firmware_update.h"
#include "gauge_firmware.h"
#include "haltech.h"
#include "hub_config.h"
#include "hub_state.h"
#include "image_store.h"
#include "intro.h"
#include "ip_bus.h"
#include "library_store.h"
#include "playback.h"
#include "recorder.h"
#include "sd_card.h"
#include "simulator.h"
#include "theme_colours.h"
#include "time_service.h"
#include "units.h"
#include "virtual_gauges.h"
#include "web_assets.h"
#include "fg_can.h"
#include "sim_presets.h"
#include "swc_output.h"
#include "wheel.h"
#include "wifi_ap.h"
#include "wifi_xfer.h"

using namespace hg;
using proto::HubManager;

namespace web_server {

namespace {

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const size_t kMaxBody = cfg::kMaxConfigBytes;
const uint16_t kMaxWsClients = 4;
// The firmware aborts on a failed allocation, so work that needs memory is
// refused or skipped while the heap is this low.
const uint32_t kMinHeapForPush = 40 * 1024;
const uint32_t kMinHeapForParse = 48 * 1024;
// Every open connection costs lwIP and AsyncTCP buffers; past this point a
// large response is more likely to crash the hub than to arrive.
const uint32_t kMinHeapForServe = 28 * 1024;

// A config being shown on a gauge with "try" must outlive the transfer.
std::vector<uint8_t> tryBuffer;

uint32_t lastStatusMs = 0;
uint32_t lastGaugesMs = 0;
uint32_t lastLiveMs = 0;
uint32_t sentRosterVersion = 0;
uint32_t lastAlertsMs = 0;
uint32_t sentAlertVersion = 0;
uint32_t lastWheelMs = 0;
uint32_t sentWheelVersion = 0;
uint32_t sentThemeVersion = 0;
uint32_t skippedPushes = 0;

// ------------------------------------------------------------ request helpers

// Request bodies are collected into request->_tempObject, which the server
// frees with the request.
struct Body {
    uint32_t cap;
    uint32_t len;
    uint8_t data[1];
};

void collectBody(AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t index, size_t total) {
    if (index == 0) {
        if (total > kMaxBody || ESP.getFreeHeap() < total + kMinHeapForParse) return;
        Body* b = static_cast<Body*>(malloc(sizeof(Body) + total));
        if (!b) return;
        b->cap = uint32_t(total);
        b->len = 0;
        b->data[0] = 0;
        r->_tempObject = b;
    }
    Body* b = static_cast<Body*>(r->_tempObject);
    if (!b || b->len + len > b->cap) return;
    memcpy(b->data + b->len, data, len);
    b->len += uint32_t(len);
    b->data[b->len] = 0;
}

const Body* bodyOf(AsyncWebServerRequest* r) { return static_cast<const Body*>(r->_tempObject); }

void sendJson(AsyncWebServerRequest* r, const JsonDocument& doc, int code = 200) {
    AsyncResponseStream* s = r->beginResponseStream("application/json");
    if (!s) {
        r->send(503);
        return;
    }
    s->setCode(code);
    serializeJson(doc, *s);
    r->send(s);
}

void sendError(AsyncWebServerRequest* r, int code, const char* message) {
    JsonDocument doc;
    doc["ok"] = false;
    doc["error"] = message;
    sendJson(r, doc, code);
}

void sendOk(AsyncWebServerRequest* r) {
    JsonDocument doc;
    doc["ok"] = true;
    sendJson(r, doc);
}

// Answers 503 and returns false when there is not enough memory to serve
// something large; browsers retry fonts and fetches on their own.
// Every open large download costs the hub about 10 KB of heap and, past two or
// three at once, the TCP stack stalls them all. Large responses count
// themselves while they are alive; further ones are refused with 503 so the
// browser retries instead of hanging.
const int kMaxBigTransfers = 2;
int bigTransfers = 0;
struct BigTransfer {
    BigTransfer() { bigTransfers++; }
    ~BigTransfer() { bigTransfers--; }
    BigTransfer(const BigTransfer&) = delete;
    BigTransfer& operator=(const BigTransfer&) = delete;
};

bool tooBusy(AsyncWebServerRequest* r) {
    if (bigTransfers < kMaxBigTransfers) return false;
    AsyncWebServerResponse* res = r->beginResponse(503, "text/plain", "hub busy, retry");
    res->addHeader("Retry-After", "1");
    r->send(res);
    return true;
}

// A block of flash sent in chunks, counting as a large transfer until done.
struct BlobCursor {
    BigTransfer guard;
    const uint8_t* data;
    size_t size;
    size_t pos = 0;
    BlobCursor(const uint8_t* d, size_t s) : data(d), size(s) {}
};

bool heapLow(AsyncWebServerRequest* r) {
    if (ESP.getFreeHeap() >= kMinHeapForServe) return false;
    AsyncWebServerResponse* res = r->beginResponse(503, "text/plain", "hub busy, retry");
    res->addHeader("Retry-After", "2");
    r->send(res);
    return true;
}

// Parses the body as JSON; an absent body counts as {}. Sends 400 on failure.
bool parseBody(AsyncWebServerRequest* r, JsonDocument& doc) {
    const Body* b = bodyOf(r);
    if (!b || b->len == 0) {
        doc.to<JsonObject>();
        return true;
    }
    if (deserializeJson(doc, reinterpret_cast<const char*>(b->data), b->len)) {
        sendError(r, 400, "body is not valid JSON");
        return false;
    }
    return true;
}

// A config in the request body, parsed with the gauge's own parser. Sends the
// error and returns false if it is missing, too big for the free memory, or
// invalid.
bool parseConfigBody(AsyncWebServerRequest* r, cfg::Config& out) {
    const Body* b = bodyOf(r);
    if (!b || b->len == 0) {
        sendError(r, 400, "send the config as the request body (up to 32 KB)");
        return false;
    }
    if (ESP.getFreeHeap() < b->len * 4 + kMinHeapForParse) {
        sendError(r, 503, "not enough free memory right now; try again in a moment");
        return false;
    }
    const cfg::ParseResult result = cfg::parseConfig(b->data, b->len, out);
    if (!result.ok) {
        sendError(r, 400, result.error.c_str());
        return false;
    }
    return true;
}

// "/api/gauges/112233/identify": segment 0 = "api", 2 = "112233", 3 = "identify".
String segment(const String& url, int index) {
    int pos = 0;
    int n = -1;
    const int len = int(url.length());
    while (pos <= len) {
        int next = url.indexOf('/', pos);
        if (next < 0) next = len;
        if (next > pos) {
            n++;
            if (n == index) return url.substring(pos, next);
        }
        pos = next + 1;
    }
    return String();
}

const char* xferName(proto::XferStatus s) {
    switch (s) {
        case proto::XferStatus::Ok: return "ok";
        case proto::XferStatus::Done: return "done";
        case proto::XferStatus::Busy: return "busy";
        case proto::XferStatus::TooBig: return "too big";
        case proto::XferStatus::BadKind: return "bad kind";
        case proto::XferStatus::CrcFail: return "crc failed";
        case proto::XferStatus::NoSession: return "no session";
        case proto::XferStatus::Rejected: return "rejected";
        case proto::XferStatus::Timeout: return "timeout";
        case proto::XferStatus::ReadFail: return "read failed";
    }
    return "?";
}

const char* kindName(proto::XferKind k) {
    switch (k) {
        case proto::XferKind::Config: return "config";
        case proto::XferKind::TryConfig: return "try";
        case proto::XferKind::SwitchConfig: return "switch";
        case proto::XferKind::WifiJoin: return "wifi";
        case proto::XferKind::Image: return "image";
    }
    return "?";
}

// ------------------------------------------------------------- JSON builders

void buildStatus(JsonObject o) {
    const HubStats s = hub::stats();
    o["uptime_s"] = millis() / 1000;
    o["fw"] = HUB_FW_VERSION;
#if HG_FG_ON_GAUGE_PORT
    o["board"] = String(hub::boardName()) + " (vehicle CAN test build)";
#else
    o["board"] = hub::boardName();
#endif
    o["reset"] = hub::resetReason();
    o["heap_free"] = ESP.getFreeHeap();
    o["heap_min"] = ESP.getMinFreeHeap();
    o["ws_clients"] = ws.count();
    o["ws_skipped"] = skippedPushes;
    o["simulator"] = s.simulator;
    JsonObject sim = o["sim"].to<JsonObject>();
    sim["paused"] = simulator::paused();
    sim["speed"] = simulator::speed();
    sim["overrides"] = simulator::overrideCount();
    recorder::statusJson(o["recording"].to<JsonObject>(), millis());
    playback::statusJson(o["playback"].to<JsonObject>());
    sd_card::statusJson(o["sd"].to<JsonObject>());
    datalog::statusJson(o["datalog"].to<JsonObject>());
    JsonObject displays = o["displays"].to<JsonObject>();
    displays["brightness"] = display_control::allBrightness();
    displays["off"] = display_control::displaysOff();
    o["gauges_online"] = s.gaugesOnline;

    JsonObject ecu = o["ecu"].to<JsonObject>();
    ecu["present"] = s.ecuPresent;
    ecu["frames"] = s.ecuFrames;
    ecu["missed"] = s.ecu.rxMissed;
    ecu["errors"] = s.ecu.busErrors;
    ecu["tec"] = s.ecu.txErrors;
    ecu["rec"] = s.ecu.rxErrors;
    ecu["state"] = s.ecu.state;

    JsonObject bus = o["bus"].to<JsonObject>();
    bus["forwarded"] = s.forwarded;
    bus["dropped"] = s.forwardDropped;
    bus["simulated"] = s.simulated;
    bus["missed"] = s.bus.rxMissed;
    bus["frames"] = s.busFrames;
    bus["errors"] = s.bus.busErrors;
    bus["rejected"] = s.bus.txRejected;
    bus["tec"] = s.bus.txErrors;
    bus["rec"] = s.bus.rxErrors;
    bus["state"] = s.bus.state;
    bus["tx_pending"] = s.txPending;

    {
        hub::Lock lock;
        HubManager& m = hub::manager();
        JsonObject push = o["push"].to<JsonObject>();
        push["active"] = m.pushBusy();
        if (m.pushBusy()) {
            char node[8];
            snprintf(node, sizeof(node), "%06lX", (unsigned long)m.pushNode());
            push["node"] = node;
            push["kind"] = kindName(m.pushKind());
            push["acked"] = m.pushFramesAcked();
            push["total"] = m.pushFramesTotal();
        }
    }
    wifi_ap::statusJson(o);
    ip_bus::statusJson(o["ip_bus"].to<JsonObject>());
    time_service::statusJson(o["time"].to<JsonObject>());
    cloud_sync::statusJson(o["cloud"].to<JsonObject>());
    firmware_update::statusJson(o["update"].to<JsonObject>());
    gauge_firmware::statusJson(o["gauge_firmware"].to<JsonObject>());
}

void buildGauges(JsonArray arr) {
    virtual_gauges::Entry planned[virtual_gauges::kMax];
    const size_t plannedCount = virtual_gauges::snapshot(planned, virtual_gauges::kMax);
    bool linked[virtual_gauges::kMax] = {};
    {
        hub::Lock lock;
        HubManager& m = hub::manager();
        const uint32_t now = millis();
        // In display order: the slot each plays in the intro, then the rest.
        size_t seq[HubManager::kMaxGauges];
        int keys[HubManager::kMaxGauges];
        int slots[HubManager::kMaxGauges];
        size_t count = 0;
        for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
            const HubManager::Gauge& g = m.gauge(i);
            if (!g.used || HubManager::isWheel(g)) continue;  // the wheel has its own card
            slots[i] = g.haveHello ? intro::slotOf(g.mac) : -1;
            keys[i] = slots[i] >= 0 ? slots[i] : 100 + int(i);
            size_t at = count;
            while (at > 0 && keys[seq[at - 1]] > keys[i]) {
                seq[at] = seq[at - 1];
                at--;
            }
            seq[at] = i;
            count++;
        }
        for (size_t s = 0; s < count; s++) {
            const size_t i = seq[s];
            const HubManager::Gauge& g = m.gauge(i);
            JsonObject o = arr.add<JsonObject>();
            char buf[24];
            snprintf(buf, sizeof(buf), "%06lX", (unsigned long)g.node);
            o["node"] = buf;
            o["virtual"] = false;
            o["link"] = ip_bus::isIpNode(g.node) ? "wifi" : "can";
            if (slots[i] >= 0) {
                o["order"] = slots[i];
            } else {
                o["order"] = nullptr;
            }
            o["online"] = g.online;
            o["last_seen_s"] = (now - g.lastSeenMs) / 1000;
            o["duplicate"] = g.duplicateId;
            if (g.haveHello) {
                snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", g.mac[0], g.mac[1], g.mac[2],
                         g.mac[3], g.mac[4], g.mac[5]);
                o["mac"] = buf;
                o["protocol"] = g.protocolVersion;
                o["schema"] = g.schemaVersion;
                char name[40];
                if (library_store::assignmentFor(g.mac, name, sizeof(name))) {
                    o["assigned"] = name;
                } else {
                    o["assigned"] = nullptr;
                }
                // A planned gauge linked to this hardware lends it its name.
                for (size_t k = 0; k < plannedCount; k++) {
                    if (planned[k].mac != buf) continue;
                    o["id"] = planned[k].id;
                    o["label"] = planned[k].label;
                    o["position"] = k;
                    linked[k] = true;
                }
            }
            if (g.haveInfo) {
                snprintf(buf, sizeof(buf), "%u.%u.%u", g.info.fwMajor, g.info.fwMinor, g.info.fwPatch);
                o["fw"] = buf;
                o["hw"] = g.info.hwRevision;
                // The hub holds a newer build for this gauge's board.
                o["fw_update"] = gauge_firmware::newerThan(g.info.hwRevision, g.info.fwMajor, g.info.fwMinor, g.info.fwPatch);
            }
            if (g.haveStatus) {
                o["face"] = g.status.activeFace;
                o["faces"] = g.status.faceCount;
                snprintf(buf, sizeof(buf), "%08lX", (unsigned long)g.status.configCrc);
                o["config_crc"] = buf;
                o["vbus_mv"] = g.status.vbusMillivolts;
                o["trying"] = (g.status.flags & proto::kStatusFlagTrying) != 0;
                o["wifi"] = (g.status.flags & proto::kStatusFlagWifi) != 0;
                o["demo"] = (g.status.flags & proto::kStatusFlagDemo) != 0;
                const int volume = proto::statusVolume(g.status.flags);
                if (volume >= 0) o["volume"] = volume;  // only a gauge with a speaker
            }
            JsonObject push = o["push"].to<JsonObject>();
            push["failed"] = g.pushFailed;
            push["result"] = xferName(g.pushResult);
            push["active"] = m.pushBusy() && m.pushNode() == g.node;
            if (push["active"]) {
                push["kind"] = kindName(m.pushKind());
                push["acked"] = m.pushFramesAcked();
                push["total"] = m.pushFramesTotal();
            }
            // What the gauge should hold: the config's CRC with its image map,
            // and how many of its images it has.
            HubManager::Desired d;
            if (g.haveHello && library_store::desired(g, d, nullptr)) {
                snprintf(buf, sizeof(buf), "%08lX", (unsigned long)d.crc);
                o["desired_crc"] = buf;
                if (d.imageCount) {
                    JsonObject images = o["images"].to<JsonObject>();
                    uint8_t have = 0, refused = 0;
                    for (uint8_t k = 0; k < d.imageCount; k++) {
                        const HubManager::ImageState s = HubManager::imageState(g, d.images[k]);
                        if (s == HubManager::ImageState::Have) have++;
                        if (s == HubManager::ImageState::Refused) refused++;
                    }
                    images["need"] = d.imageCount;
                    images["have"] = have;
                    images["refused"] = refused;
                }
            }
        }
    }

    // Planned gauges whose hardware has not announced itself.
    for (size_t k = 0; k < plannedCount; k++) {
        if (linked[k]) continue;
        JsonObject o = arr.add<JsonObject>();
        char node[8];
        snprintf(node, sizeof(node), "V%u", unsigned(planned[k].id));
        o["node"] = node;
        o["id"] = planned[k].id;
        o["virtual"] = true;
        o["label"] = planned[k].label;
        o["mac"] = planned[k].mac;
        o["online"] = false;
        o["face"] = planned[k].face;
        o["position"] = k;
        o["order"] = nullptr;
        std::string name;
        if (library_store::assignmentFor(planned[k].mac.c_str(), name)) {
            o["assigned"] = name;
        } else {
            o["assigned"] = nullptr;
        }
        JsonObject push = o["push"].to<JsonObject>();
        push["failed"] = false;
        push["result"] = "n/a";
        push["active"] = false;
    }
}

void statusMessage(String& out) {
    JsonDocument doc;
    doc["t"] = "status";
    buildStatus(doc.as<JsonObject>());
    serializeJson(doc, out);
}

void gaugesMessage(String& out) {
    JsonDocument doc;
    doc["t"] = "gauges";
    buildGauges(doc["gauges"].to<JsonArray>());
    serializeJson(doc, out);
}

// Which alerts are in force, as {"t":"alerts","states":[{"active":true,"count":2,"value":106.2},...]}.
void alertsMessage(String& out) {
    JsonDocument doc;
    doc["t"] = "alerts";
    alert_monitor::statesJson(doc["states"].to<JsonArray>());
    serializeJson(doc, out);
}

// The steering wheel module, as {"t":"wheel", ...wheel::statusJson}.
void wheelMessage(String& out) {
    JsonDocument doc;
    doc["t"] = "wheel";
    {
        hub::Lock lock;
        wheel::statusJson(doc.as<JsonObject>());
    }
    serializeJson(doc, out);
}

// The theme colours, as {"t":"themeColours","colours":[{"value":"#FFFFFF","name":"White"},...]}.
void themeColoursMessage(String& out) {
    JsonDocument doc;
    doc["t"] = "themeColours";
    theme_colours::json(doc["colours"].to<JsonArray>());
    serializeJson(doc, out);
}

// Every channel with a live value, as {"t":"live","v":{"rpm":3500,...}}.
void liveMessage(String& out) {
    const uint32_t now = millis();
    out.reserve(6144);
    out = "{\"t\":\"live\",\"v\":{";
    bool first = true;
    char buf[64];
    for (int i = 0; i < CH_COUNT; i++) {
        float v;
        if (!hub::live().get(ChannelId(i), now, v)) continue;
        snprintf(buf, sizeof(buf), "%s\"%s\":%.5g", first ? "" : ",", channelDef(ChannelId(i)).name,
                 double(v));
        out += buf;
        first = false;
    }
    out += "}}";
}

// The channel list is about 21 KB, so it is generated piece by piece as the
// connection drains rather than buffered whole.
struct MetaCursor {
    BigTransfer guard;
    enum Phase : uint8_t { Head, Units, ChannelsHead, Channels, Tail, Done };
    Phase phase = Head;
    int index = 0;
    std::string pending;

    // Appends the next piece to `pending`; false when nothing is left.
    bool next() {
        char buf[160];
        switch (phase) {
            case Head:
                pending += "{\"units\":[";
                phase = Units;
                return true;
            case Units: {
                if (index > int(Unit::Metres)) {
                    phase = ChannelsHead;
                    index = 0;
                    return next();
                }
                size_t count = 0;
                const DisplayUnit* options = displayUnits(Unit(index), count);
                snprintf(buf, sizeof(buf), "%s{\"s\":\"%s\",\"o\":[", index ? "," : "",
                         unitSymbol(Unit(index)));
                pending += buf;
                for (size_t k = 0; k < count; k++) {
                    snprintf(buf, sizeof(buf), "%s{\"n\":\"%s\",\"s\":\"%s\",\"m\":%.7g,\"a\":%.7g}",
                             k ? "," : "", options[k].name, options[k].symbol,
                             double(options[k].mul), double(options[k].add));
                    pending += buf;
                }
                pending += "]}";
                index++;
                return true;
            }
            case ChannelsHead:
                pending += "],\"channels\":[";
                phase = Channels;
                return true;
            case Channels: {
                if (index >= CH_COUNT) {
                    phase = Tail;
                    return next();
                }
                const ChannelDef& d = channelDef(ChannelId(index));
                snprintf(buf, sizeof(buf), "%s{\"n\":\"%s\",\"l\":\"%s\",\"u\":%u,\"f\":%u}",
                         index ? "," : "", d.name, d.label, unsigned(d.unit), unsigned(d.canId));
                pending += buf;
                index++;
                return true;
            }
            case Tail:
                pending += "]}";
                phase = Done;
                return true;
            case Done:
                return false;
        }
        return false;
    }
};

// ------------------------------------------------------------------ handlers

void handleIndex(AsyncWebServerRequest* r) {
    if (heapLow(r)) return;
    AsyncWebServerResponse* res = r->beginResponse(200, "text/html", WEB_INDEX_GZ, WEB_INDEX_GZ_LEN);
    res->addHeader("Content-Encoding", "gzip");
    res->addHeader("Cache-Control", "no-cache");
    r->send(res);
}

void handleFonts(AsyncWebServerRequest* r) {
    r->send(r->beginResponse(200, "application/json", WEB_FONT_MANIFEST, WEB_FONT_MANIFEST_LEN));
}

void handleFont(AsyncWebServerRequest* r) {
    if (heapLow(r)) return;
    const String name = segment(r->url(), 1);
    for (size_t i = 0; i < WEB_FONT_COUNT; i++) {
        if (name != WEB_FONTS[i].name) continue;
        if (tooBusy(r)) return;
        auto blob = std::make_shared<BlobCursor>(WEB_FONTS[i].data, WEB_FONTS[i].size);
        AsyncWebServerResponse* res =
            r->beginChunkedResponse("font/ttf", [blob](uint8_t* buf, size_t maxLen, size_t) -> size_t {
                const size_t n = std::min(maxLen, blob->size - blob->pos);
                memcpy(buf, blob->data + blob->pos, n);
                blob->pos += n;
                return n;  // 0 ends the response
            });
        res->addHeader("Cache-Control", "max-age=86400");
        r->send(res);
        return;
    }
    r->send(404);
}

void handleStatus(AsyncWebServerRequest* r) {
    JsonDocument doc;
    buildStatus(doc.to<JsonObject>());
    sendJson(r, doc);
}

void handleGauges(AsyncWebServerRequest* r) {
    JsonDocument doc;
    buildGauges(doc.to<JsonArray>());
    sendJson(r, doc);
}

// The sound a request names: "beep", "chime", "alarm" or "riff", or the number.
uint8_t soundArg(JsonVariantConst v) {
    if (v.is<int>()) {
        const int n = v.as<int>();
        return uint8_t(n < 1 ? 1 : (n > 4 ? 4 : n));
    }
    const char* s = v.as<const char*>();
    if (s && strcmp(s, "chime") == 0) return uint8_t(proto::Sound::Chime);
    if (s && strcmp(s, "alarm") == 0) return uint8_t(proto::Sound::Alarm);
    if (s && strcmp(s, "riff") == 0) return uint8_t(proto::Sound::Riff);
    return uint8_t(proto::Sound::Beep);
}

uint8_t volumeArg(JsonVariantConst v) {
    const int n = v | 60;
    return uint8_t(n < 0 ? 0 : (n > 100 ? 100 : n));
}

void handleGaugeAction(AsyncWebServerRequest* r) {
    const String nodeText = segment(r->url(), 2);
    const String action = segment(r->url(), 3);
    const uint32_t node = strtoul(nodeText.c_str(), nullptr, 16);

    if (action == "try") {
        cfg::Config parsed;
        if (!parseConfigBody(r, parsed)) return;
        if (parsed.hasImageMap) return sendError(r, 400, "'images' is filled in by the hub: leave it out");
        const Body* b = bodyOf(r);
        std::vector<uint8_t> withMap;
        std::vector<uint32_t> images;
        library_store::withImageMap(b->data, b->len, withMap, images);
        hub::Lock lock;
        HubManager& m = hub::manager();
        if (!m.find(node)) return sendError(r, 404, "unknown gauge");
        if (m.pushBusy()) return sendError(r, 409, "a transfer is already running; try again in a moment");
        tryBuffer.swap(withMap);
        // Any images the gauge lacks go first; the config follows.
        if (!m.tryConfig(node, tryBuffer.data(), uint32_t(tryBuffer.size()), millis(), images.data(),
                         images.size())) {
            return sendError(r, 409, "gauge is offline");
        }
        return sendOk(r);
    }

    if (action == "update") {
        // Takes the hub lock itself.
        std::string error;
        if (!wifi_xfer::sendFirmware(node, error)) return sendError(r, 409, error.c_str());
        return sendOk(r);
    }

    JsonDocument body;
    if (!parseBody(r, body)) return;
    hub::Lock lock;
    HubManager& m = hub::manager();
    if (segment(r->url(), 2) == "all") {
        // Every gauge at once; brightness is remembered and re-applied to newcomers.
        if (action == "brightness") {
            display_control::setAllBrightness(body["level"] | 200);
        } else if (action == "display") {
            display_control::setDisplaysOff(!(body["on"] | true));
        } else if (action == "identify") {
            display_control::sendAll(proto::Cmd::Identify, uint8_t(body["seconds"] | 5));
        } else if (action == "volume") {
            display_control::sendAll(proto::Cmd::SetVolume, volumeArg(body["level"]));
        } else if (action == "sound") {
            display_control::sendAll(proto::Cmd::PlaySound, soundArg(body["sound"]));
        } else {
            return sendError(r, 404, "unknown action for all gauges");
        }
        return sendOk(r);
    }
    if (!m.find(node)) return sendError(r, 404, "unknown gauge");
    bool ok;
    if (action == "display") {
        ok = m.command(node, proto::Cmd::SetDisplay, (body["on"] | true) ? 1 : 0);
    } else if (action == "identify") {
        ok = m.command(node, proto::Cmd::Identify, uint8_t(body["seconds"] | 5));
    } else if (action == "face") {
        ok = m.command(node, proto::Cmd::SetFace, uint8_t(body["index"] | 0));
    } else if (action == "brightness") {
        ok = m.command(node, proto::Cmd::SetBrightness, uint8_t(body["level"] | 200));
    } else if (action == "volume") {
        ok = m.command(node, proto::Cmd::SetVolume, volumeArg(body["level"]));
    } else if (action == "sound") {
        ok = m.command(node, proto::Cmd::PlaySound, soundArg(body["sound"]));
    } else if (action == "reboot") {
        ok = m.command(node, proto::Cmd::Reboot, 0);
    } else if (action == "endtry") {
        ok = m.command(node, proto::Cmd::EndTry, 0);
    } else {
        return sendError(r, 404, "unknown action");
    }
    if (!ok) return sendError(r, 409, "gauge is offline or the bus is busy");
    sendOk(r);
}

void handleChannelMeta(AsyncWebServerRequest* r) {
    if (heapLow(r) || tooBusy(r)) return;
    auto cursor = std::make_shared<MetaCursor>();
    r->sendChunked("application/json", [cursor](uint8_t* buf, size_t maxLen, size_t) -> size_t {
        size_t written = 0;
        while (written < maxLen) {
            if (cursor->pending.empty() && !cursor->next()) break;
            const size_t n = std::min(maxLen - written, cursor->pending.size());
            memcpy(buf + written, cursor->pending.data(), n);
            cursor->pending.erase(0, n);
            written += n;
        }
        return written;  // 0 ends the response
    });
}

void handleChannels(AsyncWebServerRequest* r) {
    if (heapLow(r)) return;
    String s;
    liveMessage(s);
    r->send(200, "application/json", s);
}

void handleConfigList(AsyncWebServerRequest* r) {
    JsonDocument doc;
    library_store::listJson(doc.to<JsonArray>());
    sendJson(r, doc);
}

void handleConfig(AsyncWebServerRequest* r) {
    const String name = segment(r->url(), 2);
    if (!library_store::validName(name.c_str())) return sendError(r, 400, "bad config name");
    switch (r->method()) {
        case HTTP_GET: {
            if (heapLow(r)) return;
            if (!library_store::exists(name.c_str())) return sendError(r, 404, "no such config");
            // Streamed from the file, so a 32 KB config does not need 32 KB of RAM.
            r->send(LittleFS, library_store::filePath(name.c_str()).c_str(), "application/json");
            return;
        }
        case HTTP_PUT: {
            cfg::Config parsed;
            if (!parseConfigBody(r, parsed)) return;
            const Body* b = bodyOf(r);
            std::string error;
            if (!library_store::put(name.c_str(), b->data, b->len, error)) {
                return sendError(r, 400, error.c_str());
            }
            cloud_sync::noteLocalChange(cloud_sync::Kind::Config, name.c_str());
            return sendOk(r);
        }
        case HTTP_DELETE:
            if (!library_store::remove(name.c_str())) return sendError(r, 404, "no such config");
            cloud_sync::noteLocalDelete(cloud_sync::Kind::Config, name.c_str());
            return sendOk(r);
        default:
            return sendError(r, 405, "method not allowed");
    }
}

void handleValidate(AsyncWebServerRequest* r) {
    cfg::Config parsed;
    if (!parseConfigBody(r, parsed)) return;
    JsonDocument doc;
    doc["ok"] = true;
    doc["name"] = parsed.name;
    JsonArray faces = doc["faces"].to<JsonArray>();
    size_t widgets = 0;
    for (const cfg::Face& face : parsed.faces) {
        faces.add(face.name);
        widgets += face.widgets.size();
    }
    doc["widgets"] = widgets;
    sendJson(r, doc);
}

void handleAssignments(AsyncWebServerRequest* r) {
    JsonDocument doc;
    library_store::assignmentsJson(doc.to<JsonObject>());
    sendJson(r, doc);
}

void handleAssign(AsyncWebServerRequest* r) {
    const String mac = segment(r->url(), 2);
    if (mac.length() != 17) return sendError(r, 400, "MAC must look like AA:BB:CC:DD:EE:FF");
    JsonDocument body;
    if (!parseBody(r, body)) return;
    const char* name = body["name"] | "";
    if (!library_store::assign(mac.c_str(), name)) return sendError(r, 404, "no such config");
    sendOk(r);
}

void handleTemplates(AsyncWebServerRequest* r) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    JsonDocument filter;
    filter["name"] = true;
    for (size_t i = 0; i < WEB_TEMPLATE_COUNT; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["name"] = WEB_TEMPLATES[i].name;
        JsonDocument t;
        if (!deserializeJson(t, reinterpret_cast<const char*>(WEB_TEMPLATES[i].data),
                             WEB_TEMPLATES[i].size, DeserializationOption::Filter(filter))) {
            o["title"] = t["name"] | "";
        }
    }
    sendJson(r, doc);
}

void handleTemplate(AsyncWebServerRequest* r) {
    const String name = segment(r->url(), 2);
    for (size_t i = 0; i < WEB_TEMPLATE_COUNT; i++) {
        if (name == WEB_TEMPLATES[i].name) {
            r->send(r->beginResponse(200, "application/json", WEB_TEMPLATES[i].data,
                                     WEB_TEMPLATES[i].size));
            return;
        }
    }
    sendError(r, 404, "no such template");
}

// Planned gauges: POST /api/virtual {label, mac?}, PUT /api/virtual/{id}
// {label?, face?, mac?}, DELETE /api/virtual/{id}, GET /api/virtual.
void handleVirtual(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_GET) {
        JsonDocument doc;
        virtual_gauges::listJson(doc.to<JsonArray>());
        return sendJson(r, doc);
    }
    JsonDocument body;
    if (!parseBody(r, body)) return;
    if (r->method() == HTTP_POST) {
        uint8_t id = 0;
        if (!virtual_gauges::add(body["label"] | "", id)) {
            return sendError(r, 409, "no room: at most 8 planned gauges");
        }
        const char* mac = body["mac"] | "";
        if (mac[0] && !virtual_gauges::setMac(id, mac)) {
            virtual_gauges::remove(id);
            return sendError(r, 400, "MAC must look like AA:BB:CC:DD:EE:FF and not already be linked");
        }
        hub::rosterTouched();
        JsonDocument doc;
        doc["ok"] = true;
        doc["id"] = id;
        return sendJson(r, doc);
    }
    const uint8_t id = uint8_t(segment(r->url(), 2).toInt());
    if (r->method() == HTTP_DELETE) {
        if (!virtual_gauges::remove(id)) return sendError(r, 404, "no such planned gauge");
        hub::rosterTouched();
        return sendOk(r);
    }
    bool found = true;
    if (body["label"].is<const char*>()) found = virtual_gauges::setLabel(id, body["label"]) && found;
    if (body["face"].is<int>()) found = virtual_gauges::setFace(id, uint8_t(body["face"].as<int>())) && found;
    if (body["order"].is<int>()) {
        const int at = body["order"].as<int>();
        found = virtual_gauges::move(id, size_t(at < 0 ? 0 : at)) && found;
    }
    if (!found) return sendError(r, 404, "no such planned gauge");
    if (body["mac"].is<const char*>() && !virtual_gauges::setMac(id, body["mac"])) {
        return sendError(r, 400, "MAC must look like AA:BB:CC:DD:EE:FF and not already be linked");
    }
    hub::rosterTouched();
    sendOk(r);
}

void simulatorJson(JsonDocument& doc) {
    doc["enabled"] = hub::simulatorEnabled();
    doc["paused"] = simulator::paused();
    doc["speed"] = simulator::speed();
    simulator::overridesJson(doc["overrides"].to<JsonArray>());
}

// GET returns the simulator's settings. POST changes any of them: enabled,
// paused, speed, clear, and overrides as {channel: {hold: v} | {min, max} | null}.
// ---- images: GET /api/images lists them; GET, PUT (the file as the body)
// and DELETE /api/images/{name} for one.

File imageUpload;
struct ImageMark {  // malloc'd, so the server's free() of _tempObject is right
    bool ok;
    char error[96];
};

void imageBody(AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t index, size_t total) {
    if (index == 0) {
        ImageMark* m = static_cast<ImageMark*>(malloc(sizeof(ImageMark)));
        if (!m) return;
        m->ok = false;
        m->error[0] = 0;
        r->_tempObject = m;
        std::string error;
        if (imageUpload) {
            snprintf(m->error, sizeof(m->error), "another image is uploading; try again in a moment");
            return;
        }
        if (!image_store::roomFor(uint32_t(total), error)) {
            snprintf(m->error, sizeof(m->error), "%s", error.c_str());
            return;
        }
        imageUpload = LittleFS.open(image_store::uploadPath(), "w");
        if (!imageUpload) return;
        m->ok = true;
    }
    ImageMark* m = static_cast<ImageMark*>(r->_tempObject);
    if (!m || !m->ok) return;
    if (imageUpload.write(data, len) != len) {
        m->ok = false;
        snprintf(m->error, sizeof(m->error), "not enough room on the hub");
        imageUpload.close();
        return;
    }
    if (index + len >= total) imageUpload.close();
}

void handleImages(AsyncWebServerRequest* r) {
    JsonDocument doc;
    image_store::listJson(doc["images"].to<JsonArray>());
    doc["free"] = LittleFS.totalBytes() - LittleFS.usedBytes();
    doc["total"] = LittleFS.totalBytes();
    doc["max_bytes"] = image_store::kMaxBytes;
    doc["max_count"] = image_store::kMaxCount;
    sendJson(r, doc);
}

void handleImage(AsyncWebServerRequest* r) {
    const String name = segment(r->url(), 2);
    if (r->method() == HTTP_PUT) {
        ImageMark* m = static_cast<ImageMark*>(r->_tempObject);
        if (imageUpload) imageUpload.close();
        if (!m || !m->ok) {
            LittleFS.remove(image_store::uploadPath());
            return sendError(r, 400, m && m->error[0] ? m->error : "send the image file as the request body");
        }
        std::string error;
        if (!image_store::accept(name.c_str(), error)) return sendError(r, 400, error.c_str());
        cloud_sync::noteLocalChange(cloud_sync::Kind::Image, name.c_str());
        return sendOk(r);
    }
    if (r->method() == HTTP_DELETE) {
        if (!image_store::remove(name.c_str())) return sendError(r, 404, "no such image");
        cloud_sync::noteLocalDelete(cloud_sync::Kind::Image, name.c_str());
        return sendOk(r);
    }
    std::string path;
    const char* type = nullptr;
    if (!image_store::file(name.c_str(), path, type)) return sendError(r, 404, "no such image");
    if (heapLow(r) || tooBusy(r)) return;
    auto guard = std::make_shared<BigTransfer>();
    AsyncWebServerResponse* res = r->beginResponse(LittleFS, path.c_str(), type);
    res->addHeader("Cache-Control", "no-cache");
    r->onDisconnect([guard]() {});  // counts as a large transfer until the client has it
    r->send(res);
}

// ---- recordings: files streamed to and from the filesystem

const size_t kMaxRecordingBytes = 1024 * 1024;
const char* kRecUploadTemp = "/rec/.upload.tmp";
struct UploadMark {  // malloc'd, so the server's free() of _tempObject is right
    bool ok;
    bool full;
};
File recUpload;

void recordingBody(AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t index, size_t total) {
    if (index == 0) {
        UploadMark* m = static_cast<UploadMark*>(malloc(sizeof(UploadMark)));
        if (!m) return;
        m->ok = false;
        m->full = false;
        r->_tempObject = m;
        if (total > kMaxRecordingBytes || recUpload) return;
        if (LittleFS.totalBytes() - LittleFS.usedBytes() < total + 16384) { m->full = true; return; }
        recUpload = LittleFS.open(kRecUploadTemp, "w");
        if (!recUpload) return;
        m->ok = true;
    }
    UploadMark* m = static_cast<UploadMark*>(r->_tempObject);
    if (!m || !m->ok) return;
    if (recUpload.write(data, len) != len) {
        m->ok = false;
        m->full = true;
        recUpload.close();
        return;
    }
    if (index + len >= total) recUpload.close();
}

void handleRecordings(AsyncWebServerRequest* r) {
    JsonDocument doc;
    recorder::listJson(doc.to<JsonArray>());
    sendJson(r, doc);
}

void handleRecording(AsyncWebServerRequest* r) {
    const String name = segment(r->url(), 2);
    if (r->method() == HTTP_PUT) {
        UploadMark* m = static_cast<UploadMark*>(r->_tempObject);
        if (recUpload) recUpload.close();
        if (!m || !m->ok) {
            LittleFS.remove(kRecUploadTemp);
            return sendError(r, 400, m && m->full ? "not enough storage on the hub" : "upload refused: over 1 MB, or another upload is running");
        }
        std::string err;
        if (!recorder::accept(kRecUploadTemp, name.c_str(), err)) return sendError(r, 400, err.c_str());
        return sendOk(r);
    }
    if (!recorder::exists(name.c_str())) return sendError(r, 404, "no such recording");
    if (r->method() == HTTP_DELETE) {
        if (strcmp(playback::currentName(), name.c_str()) == 0) playback::stop();
        if (!recorder::remove(name.c_str())) return sendError(r, 409, "that recording is in use");
        return sendOk(r);
    }
    r->send(LittleFS, recorder::filePath(name.c_str()).c_str(), "application/json");
}

// POST {action: "start", name, seconds} or {action: "stop"}; answers with the recorder's state.
void handleRecord(AsyncWebServerRequest* r) {
    JsonDocument body;
    if (!parseBody(r, body)) return;
    const String action = body["action"] | "";
    if (action == "start") {
        std::string err;
        if (!recorder::start(body["name"] | "", uint32_t(body["seconds"] | 60), err)) return sendError(r, 400, err.c_str());
    } else if (action == "stop") {
        recorder::stop();
    } else {
        return sendError(r, 400, "action is start or stop");
    }
    JsonDocument doc;
    recorder::statusJson(doc.to<JsonObject>(), millis());
    sendJson(r, doc);
}

// POST {action: "start", name, loop} or {action: "stop"}; answers with the playback state.
void handlePlayback(AsyncWebServerRequest* r) {
    JsonDocument body;
    if (!parseBody(r, body)) return;
    const String action = body["action"] | "";
    if (action == "start") {
        std::string err;
        if (!playback::start(body["name"] | "", body["loop"] | false, err)) return sendError(r, 400, err.c_str());
    } else if (action == "stop") {
        playback::stop();
    } else {
        return sendError(r, 400, "action is start or stop");
    }
    JsonDocument doc;
    playback::statusJson(doc.to<JsonObject>());
    sendJson(r, doc);
}

void handleSimulator(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_POST) {
        JsonDocument body;
        if (!parseBody(r, body)) return;
        if (body["enabled"].is<bool>()) hub::setSimulator(body["enabled"].as<bool>());
        std::string error;
        hub::Lock lock;
        if (!simulator::applyJson(body.as<JsonVariantConst>(), error)) return sendError(r, 400, error.c_str());
    }
    JsonDocument doc;
    simulatorJson(doc);
    sendJson(r, doc);
}

// Simulator presets (docs/web-ui.md): GET the list; PUT /api/simulator/presets/<name>
// with {speed, paused, overrides}, or an empty body for the settings in force;
// POST .../<name>/apply; DELETE .../<name>.
void handleSimPresets(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_GET) {
        JsonDocument doc;
        sim_presets::listJson(doc.to<JsonArray>());
        return sendJson(r, doc);
    }
    const String name = segment(r->url(), 3);
    std::string error;
    if (r->method() == HTTP_DELETE) {
        if (!sim_presets::remove(name.c_str())) return sendError(r, 404, "no such preset");
        return sendOk(r);
    }
    if (r->method() == HTTP_PUT) {
        JsonDocument body;
        const Body* b = bodyOf(r);
        if (b && b->len && deserializeJson(body, b->data, b->len) != DeserializationError::Ok) {
            return sendError(r, 400, "the body is not JSON");
        }
        if (!sim_presets::save(name.c_str(), body.as<JsonVariantConst>(), error)) return sendError(r, 400, error.c_str());
        return sendOk(r);
    }
    if (segment(r->url(), 4) != "apply") return sendError(r, 404, "unknown action");
    if (!sim_presets::apply(name.c_str(), error)) return sendError(r, 400, error.c_str());
    JsonDocument doc;
    simulatorJson(doc);
    sendJson(r, doc);
}

// The steering wheel module (docs/steering-wheel.md): GET its state; PUT
// settings or calibration; POST test, raw or fg.
void handleWheel(AsyncWebServerRequest* r) {
    const String action = segment(r->url(), 2);
    std::string error;
    if (r->method() == HTTP_GET) {
        JsonDocument doc;
        {
            hub::Lock lock;
            wheel::statusJson(doc.to<JsonObject>());
        }
        return sendJson(r, doc);
    }
    if (r->method() == HTTP_PUT && action == "calibration") {
        const Body* b = bodyOf(r);
        if (!b || b->len == 0) return sendError(r, 400, "send the calibration JSON as the request body");
        hub::Lock lock;
        if (!wheel::setCalibration(b->data, b->len, error)) return sendError(r, 400, error.c_str());
        return sendOk(r);
    }
    JsonDocument body;
    if (!parseBody(r, body)) return;
    hub::Lock lock;
    bool ok = false;
    if (r->method() == HTTP_PUT && action == "settings") {
        ok = wheel::setSettings(body.as<JsonVariantConst>(), error);
        if (ok && !body["fg"].isNull()) ok = fg_can::setSettings(body["fg"], error);
        if (ok && !body["swc"].isNull()) ok = swc_output::setSettings(body["swc"], error);
        if (!ok) return sendError(r, 400, error.c_str());
        return sendOk(r);
    }
    if (r->method() != HTTP_POST) return sendError(r, 404, "unknown action");
    if (action == "test") {
        ok = wheel::test(body["output"] | "", uint32_t(body["ms"] | 200), error);
    } else if (action == "raw") {
        ok = wheel::requestRaw(uint8_t(body["seconds"] | 30), error);
    } else if (action == "fg") {
        if (!body["listen"].isNull()) {
            ok = fg_can::setListen(body["listen"].as<bool>(), error);
        } else if (!body["probe"].isNull()) {
            JsonVariantConst v = body["probe"];
            const uint32_t id = v.is<const char*>() ? uint32_t(strtoul(v.as<const char*>(), nullptr, 0)) : v.as<uint32_t>();
            ok = fg_can::probe(id, body["ext"] | false, error);
        } else {
            error = "send {\"listen\": true|false} or {\"probe\": \"0x437\"} (0 stops)";
        }
    } else {
        return sendError(r, 404, "unknown action");
    }
    if (!ok) return sendError(r, 409, error.c_str());
    sendOk(r);
}

// GET gives the alert rules and their states. PUT replaces the rules with the
// list in the body, refusing the lot if any rule is wrong.
void handleAlerts(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_PUT) {
        const Body* b = bodyOf(r);
        if (!b || b->len == 0) return sendError(r, 400, "send the list of alerts as the request body");
        std::string error;
        if (!alert_monitor::setRules(b->data, b->len, error)) return sendError(r, 400, error.c_str());
        cloud_sync::noteLocalChange(cloud_sync::Kind::Alerts, nullptr);
        return sendOk(r);
    }
    JsonDocument doc;
    alert_monitor::rulesJson(doc["rules"].to<JsonArray>());
    alert_monitor::statesJson(doc["states"].to<JsonArray>());
    doc["max"] = alert_monitor::kMaxRules;
    sendJson(r, doc);
}

// GET gives the eight theme colours; PUT replaces them with the list in the body.
void handleThemeColours(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_PUT) {
        const Body* b = bodyOf(r);
        if (!b || b->len == 0) return sendError(r, 400, "send the eight theme colours as the request body");
        std::string error;
        if (!theme_colours::set(b->data, b->len, error)) return sendError(r, 400, error.c_str());
        cloud_sync::noteLocalChange(cloud_sync::Kind::ThemeColours, nullptr);
        return sendOk(r);
    }
    JsonDocument doc;
    theme_colours::json(doc["colours"].to<JsonArray>());
    sendJson(r, doc);
}

// POST {index, seconds?}: puts one alert in force for a few seconds to see what it does.
void handleAlertTest(AsyncWebServerRequest* r) {
    JsonDocument body;
    if (!parseBody(r, body)) return;
    if (!alert_monitor::test(size_t(body["index"] | -1), uint32_t(body["seconds"] | 5))) {
        return sendError(r, 404, "no such alert");
    }
    sendOk(r);
}

// GET /api/cloud is the link's state; POST /api/cloud/link {code}, /api/cloud/unlink
// and /api/cloud/sync ask the sync task to act, and the state shows the outcome.
void handleCloud(AsyncWebServerRequest* r) {
    const String action = segment(r->url(), 2);
    if (r->method() == HTTP_POST) {
        JsonDocument body;
        if (!parseBody(r, body)) return;
        if (action == "link") {
            String code = body["code"] | "";
            code.trim();
            code.toUpperCase();
            if (code.length() != 6) return sendError(r, 400, "the code is six characters");
            cloud_sync::requestLink(code.c_str());
        } else if (action == "unlink") {
            cloud_sync::requestUnlink();
        } else if (action == "sync") {
            if (!cloud_sync::linked()) return sendError(r, 409, "not linked");
            cloud_sync::requestSync();
        } else {
            return sendError(r, 404, "unknown action");
        }
    }
    JsonDocument doc;
    cloud_sync::statusJson(doc.to<JsonObject>());
    sendJson(r, doc);
}

// A firmware image uploaded from the page, streamed into the spare app slot
// as it arrives (docs/firmware-updates.md).
struct UpdateMark {
    bool ok;
    char error[96];
};

void updateBody(AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t index, size_t total) {
    if (index == 0) {
        UpdateMark* m = static_cast<UpdateMark*>(malloc(sizeof(UpdateMark)));
        if (!m) return;
        m->ok = false;
        m->error[0] = 0;
        r->_tempObject = m;
        std::string error;
        if (!firmware_update::uploadBegin(total, error)) {
            snprintf(m->error, sizeof(m->error), "%s", error.c_str());
            return;
        }
        m->ok = true;
    }
    UpdateMark* m = static_cast<UpdateMark*>(r->_tempObject);
    if (!m || !m->ok) return;
    if (!firmware_update::uploadWrite(data, len)) {
        m->ok = false;
        snprintf(m->error, sizeof(m->error), "writing the image failed; see the Firmware card");
    }
}

void handleUpdate(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_PUT) {
        UpdateMark* m = static_cast<UpdateMark*>(r->_tempObject);
        if (!m) return sendError(r, 400, "no file was sent");
        if (!m->ok) {
            firmware_update::uploadAbort();
            return sendError(r, 400, m->error[0] ? m->error : "the upload failed");
        }
        std::string error;
        if (!firmware_update::uploadEnd(error)) return sendError(r, 500, error.c_str());
        return sendOk(r);
    }
    if (r->method() == HTTP_POST) {
        const String action = segment(r->url(), 2);
        if (action == "check") {
            firmware_update::requestCheck();
        } else if (action == "install") {
            std::string error;
            if (!firmware_update::requestInstall(error)) return sendError(r, 409, error.c_str());
        } else {
            return sendError(r, 404, "unknown action");
        }
    }
    JsonDocument doc;
    firmware_update::statusJson(doc.to<JsonObject>());
    sendJson(r, doc);
}

// The key-on intro: its settings, and playing it now (docs/startup-animation.md).
void handleIntro(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_POST) {
        if (segment(r->url(), 2) != "play") return sendError(r, 404, "unknown action");
        JsonDocument doc;
        doc["ok"] = true;
        doc["gauges"] = intro::play();
        return sendJson(r, doc);
    }
    if (r->method() == HTTP_PUT) {
        JsonDocument body;
        if (!parseBody(r, body)) return;
        std::string error;
        if (!intro::setSettings(body.as<JsonObjectConst>(), error)) return sendError(r, 400, error.c_str());
    }
    JsonDocument doc;
    intro::settingsJson(doc.to<JsonObject>());
    sendJson(r, doc);
}

// Downloads by a gauge over the hub's Wi-Fi, with the token it was given (docs/images.md).
void handleGaugeImage(AsyncWebServerRequest* r) {
    if (heapLow(r)) return;
    const String hex = segment(r->url(), 2);
    const String token = r->hasParam("token") ? r->getParam("token")->value() : String();
    if (hex.length() != 8) return sendError(r, 404, "no such image");
    const uint32_t crc = strtoul(hex.c_str(), nullptr, 16);
    if (!wifi_xfer::allowsImage(token.c_str(), crc)) return sendError(r, 403, "no token for this image");
    std::string path;
    const char* type = nullptr;
    if (!image_store::fileByCrc(crc, path, type)) return sendError(r, 404, "no such image");
    r->send(LittleFS, path.c_str(), type);
}

void handleGaugeFirmwareFile(AsyncWebServerRequest* r) {
    if (heapLow(r)) return;
    const String token = r->hasParam("token") ? r->getParam("token")->value() : String();
    std::string path;
    if (!wifi_xfer::allowsFirmware(token.c_str(), path)) return sendError(r, 403, "no token for the firmware");
    r->send(LittleFS, path.c_str(), "application/octet-stream");
}

// The gauge firmware builds the hub keeps (docs/firmware-updates.md, "Gauges").
void handleGaugeFirmware(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_POST) {
        const String action = segment(r->url(), 2);
        if (action == "check") {
            gauge_firmware::requestCheck();
        } else if (action == "fetch" || action == "remove") {
            JsonDocument body;
            if (!parseBody(r, body)) return;
            const uint8_t board = uint8_t(body["board"] | 0);
            std::string error;
            if (action == "fetch" && !gauge_firmware::requestFetch(board, error)) return sendError(r, 409, error.c_str());
            if (action == "remove" && !gauge_firmware::remove(board)) {
                return sendError(r, 409, "nothing to remove, or a fetch is running");
            }
        } else {
            return sendError(r, 404, "unknown action");
        }
    }
    JsonDocument doc;
    gauge_firmware::statusJson(doc.to<JsonObject>());
    sendJson(r, doc);
}

// The hub's time of day and zone (docs/time-of-day.md).
void handleTime(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_PUT) {
        JsonDocument body;
        if (!parseBody(r, body)) return;
        if (!body["tz"].isNull()) {
            std::string error;
            if (!time_service::setZone(body["tz"] | "", body["tz_name"] | "", error)) return sendError(r, 400, error.c_str());
        }
        if (!body["unix"].isNull()) {
            if (!time_service::setFromDevice(uint32_t(body["unix"].as<uint64_t>()))) {
                return sendError(r, 409, "the hub already has the time from NTP");
            }
        }
    }
    JsonDocument doc;
    time_service::statusJson(doc.to<JsonObject>());
    sendJson(r, doc);
}

// The SD card and the datalog on it (docs/sd-card.md).
void handleSd(AsyncWebServerRequest* r) {
    const String sub = segment(r->url(), 2);
    if (sub == "logs") {
        const String name = segment(r->url(), 3);
        if (!datalog::validName(name.c_str())) return sendError(r, 400, "bad log name");
        fs::FS* fs = sd_card::fs();
        if (!fs) return sendError(r, 404, "no card");
        if (r->method() == HTTP_DELETE) {
            if (!datalog::remove(name.c_str())) return sendError(r, 409, "no such log, or it is being written");
            return sendOk(r);
        }
        if (heapLow(r) || tooBusy(r)) return;
        const std::string path = datalog::path(name.c_str());
        if (!fs->exists(path.c_str())) return sendError(r, 404, "no such log");
        r->send(*fs, path.c_str(), "text/csv", true);
        return;
    }
    if (r->method() == HTTP_POST) {
        if (sub == "mount") {
            datalog::requestMount();  // on the log's task; the status says how it went
        } else if (sub == "logging") {
            JsonDocument body;
            if (!parseBody(r, body)) return;
            datalog::setEnabled(body["on"] | true);
        } else {
            return sendError(r, 404, "unknown action");
        }
    }
    JsonDocument doc;
    sd_card::statusJson(doc["card"].to<JsonObject>());
    datalog::statusJson(doc["datalog"].to<JsonObject>());
    datalog::listJson(doc["logs"].to<JsonArray>());
    sendJson(r, doc);
}

void handleWifi(AsyncWebServerRequest* r) {
    switch (r->method()) {
        case HTTP_GET: {
            JsonDocument doc;
            wifi_ap::statusJson(doc.to<JsonObject>());
            sendJson(r, doc);
            return;
        }
        case HTTP_PUT: {
            JsonDocument body;
            if (!parseBody(r, body)) return;
            const char* ssid = body["ssid"] | "";
            if (!ssid[0]) return sendError(r, 400, "ssid is required");
            wifi_ap::setStation(ssid, body["password"] | "");
            return sendOk(r);
        }
        case HTTP_DELETE:
            wifi_ap::setStation("", "");
            return sendOk(r);
        default:
            return sendError(r, 405, "method not allowed");
    }
}

void handleNotFound(AsyncWebServerRequest* r) {
    if (r->url().startsWith("/api/")) return sendError(r, 404, "no such endpoint");
    // Captive-portal probes and mistyped addresses all land on the page, on
    // whichever of the hub's addresses the client used.
    r->redirect("/");
}

void onWsEvent(AsyncWebSocket*, AsyncWebSocketClient* client, AwsEventType type, void*, uint8_t*,
               size_t) {
    if (type != WS_EVT_CONNECT) return;
    String s;
    statusMessage(s);
    client->text(s);
    gaugesMessage(s);
    client->text(s);
    liveMessage(s);
    client->text(s);
    alertsMessage(s);
    client->text(s);
    s = String();
    themeColoursMessage(s);
    client->text(s);
    s = String();
    wheelMessage(s);
    client->text(s);
}

// Pushes to every client unless a client is backed up or memory is short;
// a missed update is harmless, a failed allocation is not.
bool pushAll(const String& s) {
    if (!ws.availableForWriteAll() || ESP.getFreeHeap() < kMinHeapForPush + s.length() * 2) {
        skippedPushes++;
        return false;
    }
    ws.textAll(s);
    return true;
}

}  // namespace

void begin() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, handleIndex);
    server.on("/index.html", HTTP_GET, handleIndex);
    server.on("/fonts/*", HTTP_GET, handleFont);

    // A handler registered for "/a/b" also matches "/a/b/anything" in this
    // library, so the item routes go before their collection routes.
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/gauges/*", HTTP_POST, handleGaugeAction, nullptr, collectBody);
    server.on("/api/gauges", HTTP_GET, handleGauges);
    server.on("/api/channels/meta", HTTP_GET, handleChannelMeta);
    server.on("/api/channels", HTTP_GET, handleChannels);
    server.on("/api/configs/*", HTTP_GET | HTTP_PUT | HTTP_DELETE, handleConfig, nullptr, collectBody);
    server.on("/api/configs", HTTP_GET, handleConfigList);
    server.on("/api/validate", HTTP_POST, handleValidate, nullptr, collectBody);
    server.on("/api/assignments/*", HTTP_PUT, handleAssign, nullptr, collectBody);
    server.on("/api/assignments", HTTP_GET, handleAssignments);
    server.on("/api/templates/*", HTTP_GET, handleTemplate);
    server.on("/api/templates", HTTP_GET, handleTemplates);
    server.on("/api/fonts", HTTP_GET, handleFonts);
    server.on("/api/virtual/*", HTTP_PUT | HTTP_DELETE, handleVirtual, nullptr, collectBody);
    server.on("/api/virtual", HTTP_GET | HTTP_POST, handleVirtual, nullptr, collectBody);
    server.on("/api/images/*", HTTP_GET | HTTP_DELETE, handleImage);
    server.on("/api/images/*", HTTP_PUT, handleImage, nullptr, imageBody);
    server.on("/api/images", HTTP_GET, handleImages);
    server.on("/api/recordings/*", HTTP_GET | HTTP_DELETE, handleRecording);
    server.on("/api/recordings/*", HTTP_PUT, handleRecording, nullptr, recordingBody);
    server.on("/api/recordings", HTTP_GET, handleRecordings);
    server.on("/api/record", HTTP_POST, handleRecord, nullptr, collectBody);
    server.on("/api/playback", HTTP_POST, handlePlayback, nullptr, collectBody);
    server.on("/api/simulator/presets/*", HTTP_PUT | HTTP_POST | HTTP_DELETE, handleSimPresets, nullptr, collectBody);
    server.on("/api/simulator/presets", HTTP_GET, handleSimPresets);
    server.on("/api/simulator", HTTP_GET | HTTP_POST, handleSimulator, nullptr, collectBody);
    server.on("/api/alerts/test", HTTP_POST, handleAlertTest, nullptr, collectBody);
    server.on("/api/theme-colours", HTTP_GET | HTTP_PUT, handleThemeColours, nullptr, collectBody);
    server.on("/api/alerts", HTTP_GET | HTTP_PUT, handleAlerts, nullptr, collectBody);
    server.on("/api/wheel/*", HTTP_PUT | HTTP_POST, handleWheel, nullptr, collectBody);
    server.on("/api/wheel", HTTP_GET, handleWheel);
    server.on("/api/wifi", HTTP_GET | HTTP_PUT | HTTP_DELETE, handleWifi, nullptr, collectBody);
    server.on("/api/cloud/*", HTTP_POST, handleCloud, nullptr, collectBody);
    server.on("/api/cloud", HTTP_GET, handleCloud);
    server.on("/gauge/image/*", HTTP_GET, handleGaugeImage);
    server.on("/gauge/firmware", HTTP_GET, handleGaugeFirmwareFile);
    server.on("/api/gauge-firmware/*", HTTP_POST, handleGaugeFirmware, nullptr, collectBody);
    server.on("/api/gauge-firmware", HTTP_GET, handleGaugeFirmware);
    server.on("/api/time", HTTP_GET | HTTP_PUT, handleTime, nullptr, collectBody);
    server.on("/api/sd/logs/*", HTTP_GET | HTTP_DELETE, handleSd);
    server.on("/api/sd/*", HTTP_POST, handleSd, nullptr, collectBody);
    server.on("/api/sd", HTTP_GET, handleSd);
    server.on("/api/intro/*", HTTP_POST, handleIntro);
    server.on("/api/intro", HTTP_GET | HTTP_PUT, handleIntro, nullptr, collectBody);
    server.on("/api/update/*", HTTP_POST, handleUpdate);
    server.on("/api/update", HTTP_GET | HTTP_PUT, handleUpdate, nullptr, updateBody);
    server.onNotFound(handleNotFound);

    server.begin();
}

void loop() {
    const uint32_t now = millis();
    ws.cleanupClients(kMaxWsClients);
    if (ws.count() == 0) {
        sentRosterVersion = hub::rosterVersion();
        sentAlertVersion = alert_monitor::version();
        sentThemeVersion = theme_colours::version();
        return;
    }
    const uint32_t themeVersion = theme_colours::version();
    if (themeVersion != sentThemeVersion) {
        String s;
        themeColoursMessage(s);
        if (pushAll(s)) sentThemeVersion = themeVersion;
    }
    // Promptly when an alert fires or ends; otherwise once a second, for the readings.
    const uint32_t alertVersion = alert_monitor::version();
    if ((alertVersion != sentAlertVersion && now - lastAlertsMs >= 100) ||
        (alert_monitor::ruleCount() && now - lastAlertsMs >= 1000)) {
        lastAlertsMs = now;
        String s;
        alertsMessage(s);
        if (pushAll(s)) sentAlertVersion = alertVersion;
    }
    if (now - lastStatusMs >= 1000) {
        lastStatusMs = now;
        String s;
        statusMessage(s);
        pushAll(s);
    }
    // The wheel: promptly on a button, otherwise every two seconds.
    const uint32_t wheelVersion = wheel::version();
    if ((wheelVersion != sentWheelVersion && now - lastWheelMs >= 100) || now - lastWheelMs >= 2000) {
        lastWheelMs = now;
        String s;
        wheelMessage(s);
        if (pushAll(s)) sentWheelVersion = wheelVersion;
    }
    const uint32_t version = hub::rosterVersion();
    if ((version != sentRosterVersion && now - lastGaugesMs >= 250) || now - lastGaugesMs >= 3000) {
        lastGaugesMs = now;
        String s;
        gaugesMessage(s);
        if (pushAll(s)) sentRosterVersion = version;  // otherwise retried after the pause
    }
    if (now - lastLiveMs >= 333) {
        lastLiveMs = now;
        String s;
        liveMessage(s);
        pushAll(s);
    }
}

}  // namespace web_server
