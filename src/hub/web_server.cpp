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
#include "display_control.h"
#include "face_model.h"
#include "haltech.h"
#include "hub_config.h"
#include "hub_state.h"
#include "library_store.h"
#include "playback.h"
#include "recorder.h"
#include "simulator.h"
#include "theme_colours.h"
#include "units.h"
#include "virtual_gauges.h"
#include "web_assets.h"
#include "wifi_ap.h"

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
    }
    return "?";
}

// ------------------------------------------------------------- JSON builders

void buildStatus(JsonObject o) {
    const HubStats s = hub::stats();
    o["uptime_s"] = millis() / 1000;
    o["fw"] = HUB_FW_VERSION;
    o["board"] = hub::boardName();
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
    JsonObject displays = o["displays"].to<JsonObject>();
    displays["brightness"] = display_control::allBrightness();
    displays["off"] = display_control::displaysOff();
    o["gauges_online"] = s.gaugesOnline;

    JsonObject ecu = o["ecu"].to<JsonObject>();
    ecu["present"] = s.ecuPresent;
    ecu["frames"] = s.ecuFrames;
    ecu["missed"] = s.ecu.rxMissed;
    ecu["errors"] = s.ecu.busErrors;
    ecu["state"] = s.ecu.state;

    JsonObject bus = o["bus"].to<JsonObject>();
    bus["forwarded"] = s.forwarded;
    bus["dropped"] = s.forwardDropped;
    bus["simulated"] = s.simulated;
    bus["missed"] = s.bus.rxMissed;
    bus["errors"] = s.bus.busErrors;
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
            push["acked"] = m.pushFramesAcked();
            push["total"] = m.pushFramesTotal();
        }
    }
    wifi_ap::statusJson(o);
}

void buildGauges(JsonArray arr) {
    virtual_gauges::Entry planned[virtual_gauges::kMax];
    const size_t plannedCount = virtual_gauges::snapshot(planned, virtual_gauges::kMax);
    bool linked[virtual_gauges::kMax] = {};
    {
        hub::Lock lock;
        HubManager& m = hub::manager();
        const uint32_t now = millis();
        for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
            const HubManager::Gauge& g = m.gauge(i);
            if (!g.used) continue;
            JsonObject o = arr.add<JsonObject>();
            char buf[24];
            snprintf(buf, sizeof(buf), "%06lX", (unsigned long)g.node);
            o["node"] = buf;
            o["virtual"] = false;
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
                    linked[k] = true;
                }
            }
            if (g.haveInfo) {
                snprintf(buf, sizeof(buf), "%u.%u.%u", g.info.fwMajor, g.info.fwMinor, g.info.fwPatch);
                o["fw"] = buf;
            }
            if (g.haveStatus) {
                o["face"] = g.status.activeFace;
                o["faces"] = g.status.faceCount;
                snprintf(buf, sizeof(buf), "%08lX", (unsigned long)g.status.configCrc);
                o["config_crc"] = buf;
                o["vbus_mv"] = g.status.vbusMillivolts;
                o["trying"] = (g.status.flags & proto::kStatusFlagTrying) != 0;
                o["demo"] = (g.status.flags & proto::kStatusFlagDemo) != 0;
            }
            JsonObject push = o["push"].to<JsonObject>();
            push["failed"] = g.pushFailed;
            push["result"] = xferName(g.pushResult);
            push["active"] = m.pushBusy() && m.pushNode() == g.node;
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

void handleGaugeAction(AsyncWebServerRequest* r) {
    const String nodeText = segment(r->url(), 2);
    const String action = segment(r->url(), 3);
    const uint32_t node = strtoul(nodeText.c_str(), nullptr, 16);

    if (action == "try") {
        cfg::Config parsed;
        if (!parseConfigBody(r, parsed)) return;
        const Body* b = bodyOf(r);
        hub::Lock lock;
        HubManager& m = hub::manager();
        if (!m.find(node)) return sendError(r, 404, "unknown gauge");
        if (m.pushBusy()) return sendError(r, 409, "a transfer is already running; try again in a moment");
        tryBuffer.assign(b->data, b->data + b->len);
        if (!m.tryConfig(node, tryBuffer.data(), uint32_t(tryBuffer.size()), millis())) {
            return sendError(r, 409, "gauge is offline");
        }
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
            return sendOk(r);
        }
        case HTTP_DELETE:
            if (!library_store::remove(name.c_str())) return sendError(r, 404, "no such config");
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
        hub::Lock lock;
        if (body["paused"].is<bool>()) simulator::setPaused(body["paused"].as<bool>());
        if (body["speed"].is<float>()) simulator::setSpeed(body["speed"].as<float>());
        if (body["clear"] | false) simulator::clearOverrides();
        for (JsonPairConst kv : body["overrides"].as<JsonObjectConst>()) {
            const int id = findChannel(kv.key().c_str());
            if (id < 0) return sendError(r, 400, "unknown channel in overrides");
            JsonVariantConst v = kv.value();
            bool ok = true;
            if (v.isNull()) {
                simulator::clearOverride(ChannelId(id));
            } else if (v["hold"].is<float>()) {
                ok = simulator::setOverride(ChannelId(id), true, v["hold"].as<float>(), 0, 0);
            } else if (v["min"].is<float>() && v["max"].is<float>()) {
                ok = simulator::setOverride(ChannelId(id), false, 0, v["min"].as<float>(), v["max"].as<float>());
            } else {
                return sendError(r, 400, "an override is {hold: value} or {min, max}, or null to remove it");
            }
            if (!ok) return sendError(r, 409, "at most 16 overrides");
        }
    }
    JsonDocument doc;
    simulatorJson(doc);
    sendJson(r, doc);
}

// GET gives the alert rules and their states. PUT replaces the rules with the
// list in the body, refusing the lot if any rule is wrong.
void handleAlerts(AsyncWebServerRequest* r) {
    if (r->method() == HTTP_PUT) {
        const Body* b = bodyOf(r);
        if (!b || b->len == 0) return sendError(r, 400, "send the list of alerts as the request body");
        std::string error;
        if (!alert_monitor::setRules(b->data, b->len, error)) return sendError(r, 400, error.c_str());
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
    server.on("/api/recordings/*", HTTP_GET | HTTP_DELETE, handleRecording);
    server.on("/api/recordings/*", HTTP_PUT, handleRecording, nullptr, recordingBody);
    server.on("/api/recordings", HTTP_GET, handleRecordings);
    server.on("/api/record", HTTP_POST, handleRecord, nullptr, collectBody);
    server.on("/api/playback", HTTP_POST, handlePlayback, nullptr, collectBody);
    server.on("/api/simulator", HTTP_GET | HTTP_POST, handleSimulator, nullptr, collectBody);
    server.on("/api/alerts/test", HTTP_POST, handleAlertTest, nullptr, collectBody);
    server.on("/api/theme-colours", HTTP_GET | HTTP_PUT, handleThemeColours, nullptr, collectBody);
    server.on("/api/alerts", HTTP_GET | HTTP_PUT, handleAlerts, nullptr, collectBody);
    server.on("/api/wifi", HTTP_GET | HTTP_PUT | HTTP_DELETE, handleWifi, nullptr, collectBody);
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
