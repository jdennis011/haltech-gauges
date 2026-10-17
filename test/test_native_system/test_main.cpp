// End-to-end test of the management protocol: a hub and several gauges on a
// simulated CAN bus, exercising discovery, config reconciliation, resets,
// unplugging, duplicate ids and commands.

#include <stdio.h>
#include <string.h>
#include <unity.h>

#include <array>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "alert_dispatch.h"
#include "intro_sequencer.h"
#include "alert_rules.h"
#include "channel_store.h"
#include "crc32.h"
#include "face_model.h"
#include "gauge_node.h"
#include "hub_manager.h"

using namespace hg;
using namespace hg::proto;

void setUp() {}
void tearDown() {}

namespace {

typedef std::array<uint8_t, 6> Mac;
typedef std::vector<uint8_t> Bytes;

struct Bus;

struct Port {
    Bus* bus = nullptr;
    std::deque<CanFrame> rx;
    bool connected = true;
    uint32_t txCount = 0;

    static bool send(const CanFrame& frame, void* ctx);
    XferLink link() { return {Port::send, this}; }
};

// Every frame a node sends reaches every other connected node.
struct Bus {
    std::vector<Port*> ports;
    int dropPermille = 0;
    uint32_t rng = 12345;

    void attach(Port& p) {
        p.bus = this;
        ports.push_back(&p);
    }
    void transmit(Port* from, const CanFrame& frame) {
        from->txCount++;
        if (!from->connected) return;
        for (Port* p : ports) {
            if (p == from || !p->connected) continue;
            rng = rng * 1664525u + 1013904223u;
            if (int((rng >> 8) % 1000) < dropPermille) continue;
            p->rx.push_back(frame);
        }
    }
};

bool Port::send(const CanFrame& frame, void* ctx) {
    Port* p = static_cast<Port*>(ctx);
    p->bus->transmit(p, frame);
    return true;
}

Bytes makeConfig(const char* text) { return Bytes(text, text + strlen(text)); }

struct SimGauge;
struct SimHub;
// A simulated gauge's "fetch" over Wi-Fi: copies the wanted images from the
// hub's library and queues a report of each. Defined after SimHub.
void simWifiFetch(SimGauge& g, const uint8_t* data, uint32_t size);

struct SimGauge {
    Port port;
    GaugeNode node;
    Mac mac;
    uint8_t schema = 1;
    uint32_t* now = nullptr;
    uint32_t rng = 1;

    // "Flash": survives a reboot.
    Bytes stored;
    // RAM state.
    Bytes tryConfig;
    Bytes rxBuffer;
    bool trying = false;
    XferKind lastRamKind = XferKind::TryConfig;  // the kind of the last RAM-only config
    uint8_t activeFace = 0;
    int identifyCount = 0;
    int storeCount = 0;
    int transfersOffered = 0;
    int rejectCount = 0;
    bool rejectConfigs = false;
    bool alertShown = false;
    int themeChanges = 0;
    uint32_t theme[kThemeColourCount] = {};
    std::string alertText;
    int alertChanges = 0;
    // Images in "flash", by CRC.
    std::map<uint32_t, Bytes> images;
    bool imagesSupported = true;
    int imageStores = 0;
    int imageQueries = 0;
    std::vector<std::string> storeLog;  // "image" or "config", in order
    std::vector<Intro> intros;          // key-on intros received, in order
    // Fetching over Wi-Fi: a join message carries the CRCs wanted; the "fetch"
    // copies them from the hub's library and reports each.
    struct SimHub* hub = nullptr;
    bool wifiSupported = true;  // firmware that understands WifiJoin
    bool wifiWorks = true;      // and can reach the hub
    int wifiJoins = 0;
    std::vector<uint32_t> toReport;

    static bool onCommand(Cmd cmd, uint8_t arg, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        switch (cmd) {
            case Cmd::Identify: g->identifyCount++; return true;
            case Cmd::SetFace: g->activeFace = arg; return true;
            case Cmd::EndTry: g->trying = false; return true;
            case Cmd::SendConfig: return g->node.upload(g->stored.data(), uint32_t(g->stored.size()), *g->now);
            default: return false;
        }
    }
    static void onAlert(const Alert& a, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        g->alertShown = a.show;
        g->alertText = a.text;
        g->alertChanges++;
    }
    static void onIntro(const Intro& i, void* ctx) { static_cast<SimGauge*>(ctx)->intros.push_back(i); }
    static void onThemeColours(const uint32_t colours[kThemeColourCount], void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        memcpy(g->theme, colours, sizeof(g->theme));
        g->themeChanges++;
    }
    static void onStatus(Status& s, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        s.configCrc = g->stored.empty() ? 0 : crc32(g->stored.data(), g->stored.size());
        s.activeFace = g->activeFace;
        s.faceCount = 3;
        s.vbusMillivolts = 4900;
        s.flags = g->trying ? kStatusFlagTrying : 0;
    }
    static bool onHasImage(uint32_t crc, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        g->imageQueries++;
        return g->images.count(crc) != 0;
    }
    static XferStatus onAccept(XferKind kind, uint32_t size, uint8_t** buffer, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        g->transfersOffered++;
        if (kind == XferKind::Image && !g->imagesSupported) return XferStatus::BadKind;
        if (kind == XferKind::WifiJoin && !g->wifiSupported) return XferStatus::BadKind;
        if (size > (kind == XferKind::Image ? kMaxImageBytes : 32 * 1024)) return XferStatus::TooBig;
        g->rxBuffer.assign(size ? size : 1, 0);
        *buffer = g->rxBuffer.data();
        return XferStatus::Ok;
    }
    static bool onComplete(XferKind kind, const uint8_t* data, uint32_t size, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        if (g->rejectConfigs) {
            g->rejectCount++;
            return false;
        }
        if (kind == XferKind::WifiJoin) {
            g->wifiJoins++;
            // Without Wi-Fi it joins nothing, and the hub's wait runs out.
            if (g->wifiWorks && g->hub) simWifiFetch(*g, data, size);
            return true;
        }
        if (kind == XferKind::Image) {
            g->images[crc32(data, size)] = Bytes(data, data + size);
            g->imageStores++;
            g->storeLog.push_back("image");
        } else if (kind == XferKind::TryConfig || kind == XferKind::SwitchConfig) {
            g->tryConfig.assign(data, data + size);
            g->trying = true;
            g->lastRamKind = kind;
        } else {
            g->stored.assign(data, data + size);
            g->storeCount++;
            g->storeLog.push_back("config");
            g->dropUnusedImages();
        }
        return true;
    }

    // What the firmware does on storing a config: keep only the images it uses.
    void dropUnusedImages() {
        cfg::Config parsed;
        if (!cfg::parseConfig(stored.data(), stored.size(), parsed).ok) return;  // a test's plain-text config
        for (auto it = images.begin(); it != images.end();) {
            bool used = false;
            for (const cfg::ImageRef& ref : parsed.images) used = used || ref.crc == it->first;
            it = used ? std::next(it) : images.erase(it);
        }
    }

    void boot(uint32_t nodeId) {
        trying = false;
        toReport.clear();
        tryConfig.clear();
        activeFace = 0;
        port.rx.clear();
        GaugeNode::Handler h = {};
        h.command = onCommand;
        h.status = onStatus;
        h.transfer = {onAccept, onComplete, nullptr, this};
        h.hasImage = imagesSupported ? onHasImage : nullptr;
        h.alert = onAlert;
        h.themeColours = onThemeColours;
        h.intro = onIntro;
        h.ctx = this;
        Info info;
        info.fwMajor = 1;
        info.fwMinor = 2;
        node = GaugeNode();
        node.init(port.link(), mac.data(), nodeId, info, schema, h);
    }
    void boot() { boot(GaugeNode::nodeFromMac(mac.data())); }

    void step() {
        while (!port.rx.empty()) {
            node.onFrame(port.rx.front(), *now);
            port.rx.pop_front();
        }
        node.poll(*now);
        while (!toReport.empty() && node.reportAsset(toReport.front(), true)) toReport.erase(toReport.begin());
        if (node.idConflict()) {
            // What the firmware does: pick a new random id and carry on.
            rng = rng * 1103515245u + 12345u + mac[5];
            node.setNode((rng >> 4) & 0xFFFFFE);
        }
    }
};

struct SimHub {
    Port port;
    HubManager mgr;
    uint32_t* now = nullptr;
    std::map<Mac, Bytes> assignments;
    std::map<std::string, Bytes> named;  // the library, for alerts that switch a gauge to a config
    uint8_t configSchema = 1;
    Bytes uploadBuffer;
    Bytes uploaded;
    int changes = 0;
    // The image library by CRC, and which images go with each gauge's config.
    std::map<uint32_t, Bytes> library;
    std::map<Mac, std::vector<uint32_t>> wantImages;
    const Bytes* sending = nullptr;
    int openImages = 0;
    // Wi-Fi for big images: the join message is the wanted CRCs, four bytes each.
    bool wifiOffered = false;
    int wifiJoins = 0;
    Bytes wifiPayload;

    static bool onDesired(const HubManager::Gauge& g, HubManager::Desired& out, void* ctx) {
        SimHub* h = static_cast<SimHub*>(ctx);
        Mac mac;
        memcpy(mac.data(), g.mac, 6);
        auto it = h->assignments.find(mac);
        if (it == h->assignments.end()) return false;
        out.data = it->second.data();
        out.size = uint32_t(it->second.size());
        out.crc = crc32(out.data, out.size);
        out.schema = h->configSchema;
        auto want = h->wantImages.find(mac);
        if (want != h->wantImages.end()) {
            for (uint32_t crc : want->second) out.images[out.imageCount++] = crc;
        }
        return true;
    }
    // alerts::Dispatcher::ConfigSource.
    static bool onNamed(const char* name, HubManager::Desired& out, void* ctx) {
        SimHub* h = static_cast<SimHub*>(ctx);
        auto it = h->named.find(name);
        if (it == h->named.end()) return false;
        out.data = it->second.data();
        out.size = uint32_t(it->second.size());
        out.crc = crc32(out.data, out.size);
        out.schema = h->configSchema;
        out.imageCount = 0;
        return true;
    }
    static bool onWifiJoin(const HubManager::Gauge&, const uint32_t* crcs, size_t count, const uint8_t** data,
                           uint32_t* size, uint32_t* waitMs, void* ctx) {
        SimHub* h = static_cast<SimHub*>(ctx);
        if (!h->wifiOffered) return false;
        h->wifiJoins++;
        h->wifiPayload.clear();
        for (size_t i = 0; i < count; i++) {
            for (int b = 0; b < 4; b++) h->wifiPayload.push_back(uint8_t(crcs[i] >> (8 * b)));
        }
        *data = h->wifiPayload.data();
        *size = uint32_t(h->wifiPayload.size());
        *waitMs = 2000;
        return true;
    }
    static bool readImage(uint32_t offset, uint8_t* out, uint32_t len, void* ctx) {
        const Bytes* b = static_cast<SimHub*>(ctx)->sending;
        if (!b || offset > b->size() || len > b->size() - offset) return false;
        memcpy(out, b->data() + offset, len);
        return true;
    }
    static bool onOpenImage(uint32_t crc, XferSource& source, uint32_t& size, void* ctx) {
        SimHub* h = static_cast<SimHub*>(ctx);
        auto it = h->library.find(crc);
        if (it == h->library.end()) return false;
        h->sending = &it->second;
        h->openImages++;
        source = {readImage, h};
        size = uint32_t(it->second.size());
        return true;
    }
    static void onCloseImage(void* ctx) {
        SimHub* h = static_cast<SimHub*>(ctx);
        h->sending = nullptr;
        h->openImages--;
    }
    static XferStatus onUploadAccept(XferKind, uint32_t size, uint8_t** buffer, void* ctx) {
        SimHub* h = static_cast<SimHub*>(ctx);
        h->uploadBuffer.assign(size ? size : 1, 0);
        *buffer = h->uploadBuffer.data();
        return XferStatus::Ok;
    }
    static bool onUploadComplete(XferKind, const uint8_t* data, uint32_t size, void* ctx) {
        static_cast<SimHub*>(ctx)->uploaded.assign(data, data + size);
        return true;
    }
    static void onChanged(void* ctx) { static_cast<SimHub*>(ctx)->changes++; }

    void boot() {
        port.rx.clear();
        HubManager::Handler h = {};
        h.desired = onDesired;
        h.upload = {onUploadAccept, onUploadComplete, nullptr, this};
        h.changed = onChanged;
        h.openImage = onOpenImage;
        h.closeImage = onCloseImage;
        h.wifiJoin = onWifiJoin;
        h.ctx = this;
        openImages = 0;
        mgr.init(port.link(), h, *now);
    }
    void step() {
        while (!port.rx.empty()) {
            mgr.onFrame(port.rx.front(), *now);
            port.rx.pop_front();
        }
        mgr.poll(*now);
    }

    int onlineCount() const {
        int n = 0;
        for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
            if (mgr.gauge(i).used && mgr.gauge(i).online) n++;
        }
        return n;
    }
    const HubManager::Gauge* byMac(const Mac& mac) const {
        for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
            const HubManager::Gauge& g = mgr.gauge(i);
            if (g.used && g.online && g.haveHello && memcmp(g.mac, mac.data(), 6) == 0) return &g;
        }
        return nullptr;
    }
};

void simWifiFetch(SimGauge& g, const uint8_t* data, uint32_t size) {
    for (uint32_t i = 0; i + 4 <= size; i += 4) {
        const uint32_t crc = uint32_t(data[i]) | (uint32_t(data[i + 1]) << 8) | (uint32_t(data[i + 2]) << 16) |
                             (uint32_t(data[i + 3]) << 24);
        auto it = g.hub->library.find(crc);
        if (it == g.hub->library.end()) continue;
        g.images[crc] = it->second;
        g.storeLog.push_back("wifi-image");
        g.toReport.push_back(crc);
    }
}

struct World {
    uint32_t now = 5000;
    Bus bus;
    SimHub hub;
    SimGauge g1, g2;
    bool hubRunning = true;

    World() {
        g1.mac = {0x24, 0x6F, 0x28, 0x11, 0x22, 0x33};
        g2.mac = {0x24, 0x6F, 0x28, 0x44, 0x55, 0x66};
        hub.now = g1.now = g2.now = &now;
        g1.hub = g2.hub = &hub;
        bus.attach(hub.port);
        bus.attach(g1.port);
        bus.attach(g2.port);
        hub.boot();
        g1.boot();
        g2.boot();
    }
    void run(uint32_t ms) {
        for (uint32_t i = 0; i < ms; i++) {
            now++;
            if (hubRunning) hub.step();
            g1.step();
            g2.step();
        }
    }
};

}  // namespace

void test_gauges_are_silent_without_a_hub() {
    World w;
    w.hubRunning = false;
    w.run(10000);
    TEST_ASSERT_EQUAL(0, w.g1.port.txCount);
    TEST_ASSERT_EQUAL(0, w.g2.port.txCount);
    TEST_ASSERT_FALSE(w.g1.node.hubPresent());
}

void test_gauges_go_silent_when_the_hub_disappears() {
    World w;
    w.run(2000);
    TEST_ASSERT_TRUE(w.g1.node.hubPresent());
    w.hubRunning = false;
    w.run(kBeaconTimeoutMs + 100);
    const uint32_t sent = w.g1.port.txCount;
    w.run(5000);
    TEST_ASSERT_EQUAL(sent, w.g1.port.txCount);
    TEST_ASSERT_FALSE(w.g1.node.hubPresent());
}

void test_discovery() {
    World w;
    w.run(1500);
    TEST_ASSERT_EQUAL(2, w.hub.onlineCount());
    TEST_ASSERT_TRUE(w.hub.mgr.anyOnline());

    const HubManager::Gauge* a = w.hub.byMac(w.g1.mac);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_HEX32(0x112233, a->node);
    TEST_ASSERT_TRUE(a->haveInfo);
    TEST_ASSERT_EQUAL(1, a->info.fwMajor);
    TEST_ASSERT_EQUAL(2, a->info.fwMinor);
    TEST_ASSERT_TRUE(a->haveStatus);
    TEST_ASSERT_EQUAL(3, a->status.faceCount);
    TEST_ASSERT_EQUAL(4900, a->status.vbusMillivolts);
    TEST_ASSERT_FALSE(a->duplicateId);
    TEST_ASSERT_NOT_NULL(w.hub.byMac(w.g2.mac));
    TEST_ASSERT_GREATER_THAN(0, w.hub.changes);
}

void test_assigned_config_is_pushed_once() {
    World w;
    const Bytes a = makeConfig("{\"schema\":1,\"name\":\"A\"}");
    w.hub.assignments[w.g1.mac] = a;
    w.run(3000);
    TEST_ASSERT_TRUE(w.g1.stored == a);
    TEST_ASSERT_EQUAL(1, w.g1.storeCount);
    TEST_ASSERT_TRUE(w.g2.stored.empty());  // nothing assigned to the other gauge
    TEST_ASSERT_EQUAL(0, w.g2.transfersOffered);
    TEST_ASSERT_EQUAL_HEX32(crc32(a.data(), a.size()), w.hub.byMac(w.g1.mac)->status.configCrc);

    w.run(10000);
    TEST_ASSERT_EQUAL(1, w.g1.storeCount);  // already matches, so not sent again
}

void test_edited_config_is_pushed_again() {
    World w;
    w.hub.assignments[w.g1.mac] = makeConfig("first");
    w.run(3000);
    const Bytes b(5000, 'b');  // larger than one block
    w.hub.assignments[w.g1.mac] = b;
    w.run(3000);
    TEST_ASSERT_TRUE(w.g1.stored == b);
    TEST_ASSERT_EQUAL(2, w.g1.storeCount);
}

void test_gauge_reset_keeps_or_regains_its_config() {
    World w;
    const Bytes a = makeConfig("config A");
    w.hub.assignments[w.g1.mac] = a;
    w.run(3000);

    // Power cycle with flash intact: CRC still matches, nothing is re-sent.
    w.g1.boot();
    w.run(5000);
    TEST_ASSERT_EQUAL(1, w.g1.storeCount);
    TEST_ASSERT_NOT_NULL(w.hub.byMac(w.g1.mac));

    // A replacement gauge with empty storage gets the config again.
    w.g1.stored.clear();
    w.g1.boot();
    w.run(5000);
    TEST_ASSERT_TRUE(w.g1.stored == a);
    TEST_ASSERT_EQUAL(2, w.g1.storeCount);
}

void test_hub_restart_rebuilds_roster_without_pushing() {
    World w;
    w.hub.assignments[w.g1.mac] = makeConfig("config A");
    w.run(3000);
    TEST_ASSERT_EQUAL(1, w.g1.storeCount);

    w.hub.boot();
    TEST_ASSERT_EQUAL(0, w.hub.onlineCount());
    w.run(2500);
    TEST_ASSERT_EQUAL(2, w.hub.onlineCount());
    TEST_ASSERT_NOT_NULL(w.hub.byMac(w.g1.mac));
    TEST_ASSERT_NOT_NULL(w.hub.byMac(w.g2.mac));
    w.run(5000);
    TEST_ASSERT_EQUAL(1, w.g1.storeCount);
}

void test_unplugged_gauge_goes_offline_and_returns() {
    World w;
    w.run(1500);
    w.g1.port.connected = false;
    w.run(kGaugeTimeoutMs + 200);
    TEST_ASSERT_EQUAL(1, w.hub.onlineCount());
    TEST_ASSERT_NULL(w.hub.byMac(w.g1.mac));
    TEST_ASSERT_TRUE(w.hub.mgr.anyOnline());

    w.g2.port.connected = false;
    w.run(kGaugeTimeoutMs + 200);
    TEST_ASSERT_FALSE(w.hub.mgr.anyOnline());

    w.g1.port.connected = true;
    w.run(2000);
    TEST_ASSERT_NOT_NULL(w.hub.byMac(w.g1.mac));
}

void test_unplug_during_push_recovers() {
    World w;
    const Bytes big(20000, 'x');
    w.hub.assignments[w.g1.mac] = big;
    for (int i = 0; i < 3000 && !w.hub.mgr.pushBusy(); i++) w.run(1);
    TEST_ASSERT_TRUE(w.hub.mgr.pushBusy());
    w.run(20);
    TEST_ASSERT_TRUE(w.hub.mgr.pushBusy());
    TEST_ASSERT_GREATER_THAN(0, w.hub.mgr.pushFramesAcked());

    w.g1.port.connected = false;
    w.run(8000);
    TEST_ASSERT_FALSE(w.hub.mgr.pushBusy());
    TEST_ASSERT_TRUE(w.g1.stored.empty());

    w.g1.port.connected = true;
    w.run(12000);
    TEST_ASSERT_TRUE(w.g1.stored == big);
}

void test_duplicate_node_ids_are_resolved() {
    World w;
    w.g1.boot(0xABCDEF);
    w.g2.boot(0xABCDEF);
    w.hub.assignments[w.g1.mac] = makeConfig("for gauge one");
    w.hub.assignments[w.g2.mac] = makeConfig("for gauge two");
    w.run(15000);

    TEST_ASSERT_NOT_EQUAL(w.g1.node.node(), w.g2.node.node());
    const HubManager::Gauge* a = w.hub.byMac(w.g1.mac);
    const HubManager::Gauge* b = w.hub.byMac(w.g2.mac);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_HEX32(w.g1.node.node(), a->node);
    TEST_ASSERT_EQUAL_HEX32(w.g2.node.node(), b->node);
    // Each ends up with its own config, not the other's.
    TEST_ASSERT_TRUE(w.g1.stored == makeConfig("for gauge one"));
    TEST_ASSERT_TRUE(w.g2.stored == makeConfig("for gauge two"));
}

void test_commands_reach_the_gauge_and_are_acknowledged() {
    World w;
    w.run(1500);
    const uint32_t node = w.g1.node.node();

    TEST_ASSERT_TRUE(w.hub.mgr.command(node, Cmd::Identify, 5));
    w.run(50);
    TEST_ASSERT_EQUAL(1, w.g1.identifyCount);
    TEST_ASSERT_EQUAL(0, w.g2.identifyCount);
    const HubManager::Gauge* g = w.hub.mgr.find(node);
    TEST_ASSERT_TRUE(g->haveAck);
    TEST_ASSERT_TRUE(g->ack.ok);
    TEST_ASSERT_EQUAL(int(Cmd::Identify), int(g->ack.cmd));

    TEST_ASSERT_TRUE(w.hub.mgr.command(node, Cmd::SetFace, 2));
    w.run(50);
    TEST_ASSERT_EQUAL(2, w.hub.mgr.find(node)->status.activeFace);  // reported without waiting a second

    TEST_ASSERT_TRUE(w.hub.mgr.command(node, Cmd::Reboot, 0));
    w.run(50);
    TEST_ASSERT_FALSE(w.hub.mgr.find(node)->ack.ok);  // this simulated gauge refuses it

    TEST_ASSERT_FALSE(w.hub.mgr.command(0x999999, Cmd::Identify, 1));  // unknown gauge
}

void test_alert_message_stays_up_only_while_the_hub_repeats_it() {
    World w;
    w.run(1500);
    const uint32_t node = w.g1.node.node();

    Alert a;
    a.show = true;
    a.color = 0xFFCC00;
    strcpy(a.text, "COOLANT HOT");
    TEST_ASSERT_TRUE(w.hub.mgr.alert(node, a, 1));
    w.run(20);
    TEST_ASSERT_TRUE(w.g1.alertShown);
    TEST_ASSERT_EQUAL_STRING("COOLANT HOT", w.g1.alertText.c_str());
    TEST_ASSERT_FALSE(w.g2.alertShown);

    // Repeated every second it stays up, and is not reported again.
    for (int i = 0; i < 5; i++) {
        w.run(kAlertRepeatMs);
        TEST_ASSERT_TRUE(w.hub.mgr.alert(node, a, 1));
    }
    w.run(20);
    TEST_ASSERT_TRUE(w.g1.alertShown);
    TEST_ASSERT_EQUAL(1, w.g1.alertChanges);

    // The hub stops repeating it (or its clear is lost): the gauge takes it down itself.
    w.run(3100);
    TEST_ASSERT_FALSE(w.g1.alertShown);

    // A clear takes effect at once.
    TEST_ASSERT_TRUE(w.hub.mgr.alert(node, a, 2));
    w.run(20);
    TEST_ASSERT_TRUE(w.g1.alertShown);
    TEST_ASSERT_TRUE(w.hub.mgr.alert(node, Alert(), 2));
    w.run(20);
    TEST_ASSERT_FALSE(w.g1.alertShown);

    TEST_ASSERT_FALSE(w.hub.mgr.alert(0x999999, a, 3));  // unknown gauge
}

namespace {

// The hub's alert check running alongside the bus: rules, live data, and the
// dispatcher that carries them out, stepped a millisecond at a time.
struct AlertRig {
    World& w;
    alerts::Engine engine;
    alerts::Dispatcher dispatcher;
    ChannelStore store;

    AlertRig(World& world, const char* json) : w(world) {
        std::vector<alerts::Rule> rules;
        const alerts::ParseResult r = alerts::parseRules(reinterpret_cast<const uint8_t*>(json), strlen(json), rules);
        TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
        engine.setRules(rules);
        dispatcher.setConfigSource({SimHub::onNamed, &w.hub});
    }
    void run(ChannelId id, float value, uint32_t ms) {
        for (uint32_t i = 0; i < ms; i++) {
            w.now++;
            if (w.now % 100 == 0) {
                store.set(id, value, true, w.now);
                engine.evaluate(store, w.now);
                dispatcher.run(engine, w.hub.mgr, w.now);
            }
            w.hub.step();
            w.g1.step();
            w.g2.step();
        }
    }
};

}  // namespace

void test_alert_rules_are_carried_out_on_the_gauges() {
    World w;
    w.run(1500);
    w.g1.activeFace = 1;
    w.g2.activeFace = 0;
    w.run(1500);  // the hub hears which face each is on

    // Coolant: a message on every gauge, and gauge 1 switches to face 2 and back.
    AlertRig rig(w, R"([
      {"channel":"coolant_temp","when":"above","value":105,"hold":1,"message":"COOLANT HOT"},
      {"channel":"coolant_temp","when":"above","value":105,"hold":1,"face":2,"gauge":"24:6F:28:11:22:33"}
    ])");
    rig.run(CH_coolant_temp, 90, 500);
    TEST_ASSERT_FALSE(w.g1.alertShown);
    TEST_ASSERT_EQUAL(1, w.g1.activeFace);

    rig.run(CH_coolant_temp, 110, 300);
    TEST_ASSERT_TRUE(w.g1.alertShown);
    TEST_ASSERT_TRUE(w.g2.alertShown);
    TEST_ASSERT_EQUAL_STRING("COOLANT HOT", w.g2.alertText.c_str());
    TEST_ASSERT_EQUAL(2, w.g1.activeFace);
    TEST_ASSERT_EQUAL(0, w.g2.activeFace);

    // It stays up for as long as the condition holds, without being re-announced.
    rig.run(CH_coolant_temp, 110, 6000);
    TEST_ASSERT_TRUE(w.g1.alertShown);
    TEST_ASSERT_EQUAL(1, w.g1.alertChanges);

    // A gauge that resets in the middle of it gets the message again.
    w.g2.boot();
    w.g2.alertShown = false;
    rig.run(CH_coolant_temp, 110, 2500);
    TEST_ASSERT_TRUE(w.g2.alertShown);

    // The alert ends: the message comes down and gauge 1 goes back to its face.
    rig.run(CH_coolant_temp, 90, 1500);
    TEST_ASSERT_FALSE(w.g1.alertShown);
    TEST_ASSERT_FALSE(w.g2.alertShown);
    TEST_ASSERT_EQUAL(1, w.g1.activeFace);
}

void test_alert_switches_a_gauge_to_another_config_and_back() {
    World w;
    const Bytes own = makeConfig("the gauge's own config");
    const Bytes cruise = makeConfig("the cruise config");
    w.hub.assignments[w.g1.mac] = own;
    w.hub.named["Cruise"] = cruise;
    w.run(3000);  // pushed and stored
    TEST_ASSERT_TRUE(w.g1.stored == own);
    w.g1.activeFace = 1;
    w.run(1500);  // the hub hears which face it is on

    // Cruise engages: gauge 1 shows the cruise config on its third face; gauge 2 is left alone.
    AlertRig rig(w, R"([
      {"channel":"cruise_state","when":"above","value":1,"hold":1,"config":"Cruise","face":2,"gauge":"24:6F:28:11:22:33"}
    ])");
    rig.run(CH_cruise_state, 1, 500);
    TEST_ASSERT_FALSE(w.g1.trying);
    rig.run(CH_cruise_state, 2, 2500);
    TEST_ASSERT_TRUE(w.g1.trying);
    TEST_ASSERT_TRUE(w.g1.tryConfig == cruise);
    TEST_ASSERT_EQUAL(int(XferKind::SwitchConfig), int(w.g1.lastRamKind));
    TEST_ASSERT_TRUE(w.g1.stored == own);  // not stored over its own
    TEST_ASSERT_EQUAL(2, w.g1.activeFace);
    TEST_ASSERT_FALSE(w.g2.trying);

    // While it holds the config is not sent again.
    const int offered = w.g1.transfersOffered;
    rig.run(CH_cruise_state, 3, 6000);
    TEST_ASSERT_EQUAL(offered, w.g1.transfersOffered);
    TEST_ASSERT_TRUE(w.g1.trying);

    // The gauge resets in the middle of it and comes back on its own config: switched again.
    w.g1.boot();
    rig.run(CH_cruise_state, 3, 6000);
    TEST_ASSERT_TRUE(w.g1.trying);
    TEST_ASSERT_TRUE(w.g1.tryConfig == cruise);
    TEST_ASSERT_EQUAL(2, w.g1.activeFace);

    // Cruise disengages: a second later the gauge is back on its own config and its earlier face.
    rig.run(CH_cruise_state, 1, 300);
    TEST_ASSERT_TRUE(w.g1.trying);  // the hold time
    rig.run(CH_cruise_state, 1, 2000);
    TEST_ASSERT_FALSE(w.g1.trying);
    TEST_ASSERT_TRUE(w.g1.stored == own);
    TEST_ASSERT_EQUAL(1, w.g1.activeFace);
}

void test_alert_naming_a_missing_config_does_nothing() {
    World w;
    w.hub.assignments[w.g1.mac] = makeConfig("own");
    w.run(3000);
    AlertRig rig(w, R"([{"channel":"cruise_state","when":"above","value":1,"hold":0,"config":"Nowhere","message":"CRUISE"}])");
    const int offered = w.g1.transfersOffered;
    rig.run(CH_cruise_state, 2, 4000);
    TEST_ASSERT_EQUAL(offered, w.g1.transfersOffered);
    TEST_ASSERT_FALSE(w.g1.trying);
    TEST_ASSERT_TRUE(w.g1.alertShown);  // the message still shows
    rig.run(CH_cruise_state, 0, 1500);
    TEST_ASSERT_FALSE(w.g1.alertShown);
}

void test_hub_time_reaches_the_gauges() {
    World w;
    w.run(1500);
    uint32_t seconds = 0;
    TEST_ASSERT_FALSE(w.g1.node.localTime(w.now, seconds));  // the hub has no time yet

    w.hub.mgr.setLocalTime(1760600000);
    w.run(1200);  // at least one beacon
    TEST_ASSERT_TRUE(w.g1.node.localTime(w.now, seconds));
    TEST_ASSERT_TRUE(seconds >= 1760600000 && seconds <= 1760600002);
    TEST_ASSERT_TRUE(w.g2.node.localTime(w.now, seconds));

    // Between beacons, and after the hub goes away, the gauge keeps counting.
    w.hubRunning = false;
    w.run(10000);
    TEST_ASSERT_TRUE(w.g1.node.localTime(w.now, seconds));
    TEST_ASSERT_TRUE(seconds >= 1760600010 && seconds <= 1760600012);
}

void test_intro_plays_across_the_gauges_in_order() {
    World w;
    // Gauge 2 sits left of gauge 1 on the dash: the planned order says so.
    hg::intro::Sequencer seq;
    hg::intro::Slot order[2];
    memcpy(order[0].mac, w.g2.mac.data(), 6);
    memcpy(order[1].mac, w.g1.mac.data(), 6);
    seq.setOrder(order, 2);
    seq.begin(w.now, 2);

    // Both announce themselves within a second; the intro goes as soon as both are heard.
    uint32_t waited = 0;
    while (!seq.done() && waited < 5000) {
        w.run(100);
        seq.poll(w.hub.mgr, w.now);
        waited += 100;
    }
    TEST_ASSERT_TRUE(seq.done());
    TEST_ASSERT_TRUE(waited < hg::intro::Sequencer::kSettleMs + 1000);
    w.run(50);
    TEST_ASSERT_EQUAL(1, w.g2.intros.size());
    TEST_ASSERT_EQUAL(1, w.g1.intros.size());
    TEST_ASSERT_EQUAL(0, w.g2.intros[0].index);
    TEST_ASSERT_EQUAL(0, w.g2.intros[0].delayMs);
    TEST_ASSERT_EQUAL(1, w.g1.intros[0].index);
    TEST_ASSERT_EQUAL(250, w.g1.intros[0].delayMs);
    TEST_ASSERT_EQUAL(2, w.g1.intros[0].count);
    TEST_ASSERT_EQUAL(1200, w.g1.intros[0].durationMs);
    TEST_ASSERT_FALSE(w.g1.intros[0].force);
    TEST_ASSERT_EQUAL(int(IntroStyle::Sweep), int(w.g1.intros[0].style));
    TEST_ASSERT_EQUAL(1, seq.slotOf(w.hub.mgr, w.g1.mac.data()));
    TEST_ASSERT_EQUAL(0, seq.slotOf(w.hub.mgr, w.g2.mac.data()));

    // A gauge that resets later is not told again: it plays alone on its own clock.
    w.g1.boot();
    w.g1.intros.clear();
    for (int i = 0; i < 30; i++) {
        w.run(100);
        seq.poll(w.hub.mgr, w.now);
    }
    TEST_ASSERT_EQUAL(0, w.g1.intros.size());

    // Play from the page: everyone again, forced, in the same order.
    TEST_ASSERT_EQUAL(2, seq.play(w.hub.mgr, w.now));
    w.run(50);
    TEST_ASSERT_EQUAL(1, w.g1.intros.size());
    TEST_ASSERT_TRUE(w.g1.intros[0].force);
    TEST_ASSERT_EQUAL(1, w.g1.intros[0].index);
    TEST_ASSERT_TRUE(w.g2.intros.back().force);
}

void test_intro_goes_after_a_wait_when_a_gauge_is_missing() {
    World w;
    w.g2.port.connected = false;  // one of the two expected gauges never turns up
    hg::intro::Sequencer seq;
    seq.begin(w.now, 2);
    uint32_t waited = 0;
    uint32_t firstHello = 0;
    while (!seq.done() && waited < 10000) {
        w.run(100);
        waited += 100;
        if (!firstHello && w.hub.onlineCount() > 0) firstHello = waited;
        seq.poll(w.hub.mgr, w.now);
    }
    TEST_ASSERT_TRUE(seq.done());
    TEST_ASSERT_TRUE(firstHello > 0);
    // Sent the settle time after the first hello, not at once and not never.
    TEST_ASSERT_TRUE(waited >= firstHello + hg::intro::Sequencer::kSettleMs - 100);
    TEST_ASSERT_TRUE(waited <= firstHello + hg::intro::Sequencer::kSettleMs + 300);
    w.run(50);
    TEST_ASSERT_EQUAL(1, w.g1.intros.size());
    TEST_ASSERT_EQUAL(0, w.g1.intros[0].index);
    TEST_ASSERT_EQUAL(1, w.g1.intros[0].count);

    // Switched off: the wait still ends, nothing is sent.
    World w2;
    hg::intro::Sequencer off;
    hg::intro::Settings s;
    s.enabled = false;
    off.setSettings(s);
    off.begin(w2.now, 2);
    for (int i = 0; i < 40; i++) {
        w2.run(100);
        off.poll(w2.hub.mgr, w2.now);
    }
    TEST_ASSERT_TRUE(off.done());
    TEST_ASSERT_EQUAL(0, w2.g1.intros.size());
    TEST_ASSERT_EQUAL(0, w2.g2.intros.size());
}

void test_higher_alert_takes_over_the_message() {
    World w;
    w.run(1500);
    AlertRig rig(w, R"([
      {"channel":"rpm","when":"above","value":7000,"hold":0,"message":"SHIFT","restore":false,"face":3},
      {"channel":"rpm","when":"above","value":3000,"hold":0,"message":"ON BOOST"}
    ])");
    rig.run(CH_rpm, 4000, 300);
    TEST_ASSERT_EQUAL_STRING("ON BOOST", w.g1.alertText.c_str());
    rig.run(CH_rpm, 7500, 300);
    TEST_ASSERT_EQUAL_STRING("SHIFT", w.g1.alertText.c_str());
    TEST_ASSERT_EQUAL(3, w.g1.activeFace);
    // Back under the shift point: the lower alert's message returns, and with
    // "restore" off the gauge stays on the face it was sent to.
    rig.run(CH_rpm, 4000, 300);
    TEST_ASSERT_TRUE(w.g1.alertShown);
    TEST_ASSERT_EQUAL_STRING("ON BOOST", w.g1.alertText.c_str());
    TEST_ASSERT_EQUAL(3, w.g1.activeFace);
    rig.run(CH_rpm, 1000, 300);
    TEST_ASSERT_FALSE(w.g1.alertShown);
    TEST_ASSERT_EQUAL(3, w.g1.activeFace);
}

void test_theme_colours_reach_every_gauge() {
    World w;
    TEST_ASSERT_FALSE(w.hub.mgr.themeColours(kBroadcastNode, w.g1.theme));  // nobody online yet
    w.run(1500);

    uint32_t colours[kThemeColourCount] = {0xFFFFFF, 0xFF8C00, 0xFF3B30, 0xFFCC00,
                                           0x30D158, 0x0A84FF, 0x8E8E93, 0x000000};
    TEST_ASSERT_TRUE(w.hub.mgr.themeColours(kBroadcastNode, colours));
    w.run(20);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(colours, w.g1.theme, kThemeColourCount);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(colours, w.g2.theme, kThemeColourCount);
    TEST_ASSERT_EQUAL(1, w.g1.themeChanges);

    // The regular repeat changes nothing, so the gauges are not told again.
    TEST_ASSERT_TRUE(w.hub.mgr.themeColours(kBroadcastNode, colours));
    w.run(20);
    TEST_ASSERT_EQUAL(1, w.g1.themeChanges);

    // One colour changed on the hub reaches both.
    colours[1] = 0x00AAFF;
    TEST_ASSERT_TRUE(w.hub.mgr.themeColours(kBroadcastNode, colours));
    w.run(20);
    TEST_ASSERT_EQUAL_HEX32(0x00AAFF, w.g1.theme[1]);
    TEST_ASSERT_EQUAL_HEX32(0x00AAFF, w.g2.theme[1]);
    TEST_ASSERT_EQUAL(2, w.g2.themeChanges);

    // Sent to one gauge, only that one hears it.
    colours[0] = 0x111111;
    TEST_ASSERT_TRUE(w.hub.mgr.themeColours(w.g1.node.node(), colours));
    w.run(20);
    TEST_ASSERT_EQUAL_HEX32(0x111111, w.g1.theme[0]);
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFF, w.g2.theme[0]);
}

void test_try_config_is_shown_but_not_stored() {
    World w;
    const Bytes a = makeConfig("stored config");
    const Bytes t = makeConfig("experimental config");
    const Bytes b = makeConfig("new assignment");
    w.hub.assignments[w.g1.mac] = a;
    w.run(3000);
    const uint32_t node = w.g1.node.node();

    TEST_ASSERT_TRUE(w.hub.mgr.tryConfig(node, t.data(), uint32_t(t.size()), w.now));
    w.run(1500);
    TEST_ASSERT_TRUE(w.g1.trying);
    TEST_ASSERT_TRUE(w.g1.tryConfig == t);
    TEST_ASSERT_TRUE(w.g1.stored == a);
    TEST_ASSERT_TRUE(w.hub.mgr.find(node)->status.flags & kStatusFlagTrying);

    // A changed assignment waits until the preview ends.
    w.hub.assignments[w.g1.mac] = b;
    w.run(5000);
    TEST_ASSERT_TRUE(w.g1.stored == a);

    TEST_ASSERT_TRUE(w.hub.mgr.command(node, Cmd::EndTry, 0));
    w.run(3000);
    TEST_ASSERT_FALSE(w.g1.trying);
    TEST_ASSERT_TRUE(w.g1.stored == b);
}

void test_refused_config_is_not_retried_until_the_gauge_changes() {
    World w;
    w.g1.rejectConfigs = true;
    const Bytes a = makeConfig("config the gauge refuses");
    w.hub.assignments[w.g1.mac] = a;
    w.run(30000);
    TEST_ASSERT_EQUAL(1, w.g1.rejectCount);
    const HubManager::Gauge* g = w.hub.byMac(w.g1.mac);
    TEST_ASSERT_TRUE(g->pushFailed);
    TEST_ASSERT_EQUAL(int(XferStatus::Rejected), int(g->pushResult));

    // New firmware on the gauge: it announces itself again and the push is retried.
    w.g1.rejectConfigs = false;
    w.g1.boot();
    w.run(5000);
    TEST_ASSERT_TRUE(w.g1.stored == a);
    TEST_ASSERT_FALSE(w.hub.byMac(w.g1.mac)->pushFailed);
}

void test_config_newer_than_gauge_firmware_is_not_sent() {
    World w;
    w.hub.configSchema = 2;
    w.hub.assignments[w.g1.mac] = makeConfig("needs schema 2");
    w.run(10000);
    TEST_ASSERT_EQUAL(0, w.g1.transfersOffered);
    TEST_ASSERT_TRUE(w.hub.byMac(w.g1.mac)->pushFailed);
}

void test_gauge_uploads_its_config_on_request() {
    World w;
    w.g1.stored = Bytes(3000, 'u');
    w.g1.boot();
    w.run(1500);
    TEST_ASSERT_TRUE(w.hub.mgr.requestUpload(w.g1.node.node(), w.now));
    w.run(3000);
    TEST_ASSERT_TRUE(w.hub.uploaded == w.g1.stored);
    TEST_ASSERT_FALSE(w.hub.mgr.uploadBusy());
}

void test_converges_on_a_lossy_bus() {
    World w;
    w.bus.dropPermille = 20;
    const Bytes a(4096, 'a');
    const Bytes b(4096, 'b');
    w.hub.assignments[w.g1.mac] = a;
    w.hub.assignments[w.g2.mac] = b;
    w.run(20000);
    TEST_ASSERT_TRUE(w.g1.stored == a);
    TEST_ASSERT_TRUE(w.g2.stored == b);
    TEST_ASSERT_EQUAL(2, w.hub.onlineCount());
}

void test_channel_store_staleness() {
    ChannelStore store;
    float v = 0;
    TEST_ASSERT_FALSE(store.get(CH_rpm, 1000, v));  // never received

    CanFrame f;
    f.id = 0x360;
    f.len = 8;
    f.data[0] = 0x0D;
    f.data[1] = 0xAC;
    TEST_ASSERT_EQUAL(4, store.onFrame(f, 1000));
    TEST_ASSERT_TRUE(store.get(CH_rpm, 1010, v));
    TEST_ASSERT_EQUAL_FLOAT(3500, v);

    // 50 Hz channel: stale after half a second of silence.
    TEST_ASSERT_EQUAL(500, ChannelStore::staleAfterMs(CH_rpm));
    TEST_ASSERT_TRUE(store.get(CH_rpm, 1500, v));
    TEST_ASSERT_FALSE(store.get(CH_rpm, 1501, v));
    // 5 Hz channel: allowed a full second.
    TEST_ASSERT_EQUAL(1000, ChannelStore::staleAfterMs(CH_coolant_temp));

    // A sensor error from the ECU reads as "no value".
    CanFrame tyres;
    tyres.id = 0x6F0;
    tyres.len = 8;
    store.onFrame(tyres, 2000);
    TEST_ASSERT_FALSE(store.get(CH_tyre_pressure_fl, 2000, v));

    // Timestamps keep working across the 32-bit millisecond wrap.
    store.onFrame(f, 0xFFFFFF00u);
    TEST_ASSERT_TRUE(store.get(CH_rpm, 0x00000050u, v));
    TEST_ASSERT_FALSE(store.get(CH_rpm, 0x00000200u, v));

    store.clear();
    TEST_ASSERT_FALSE(store.get(CH_rpm, 0xFFFFFF00u, v));
}

// ---------------------------------------------------------------- images

namespace {

Bytes makeImage(size_t size, uint32_t seed) {
    Bytes b(size);
    uint32_t x = seed;
    for (size_t i = 0; i < size; i++) {
        x = x * 1103515245u + 12345u;
        b[i] = uint8_t(x >> 16);
    }
    return b;
}

uint32_t crcOf(const Bytes& b) { return crc32(b.data(), b.size()); }

// A config naming its images, with the map the hub adds on the way to the gauge.
Bytes configWithImages(const char* name, const std::vector<std::pair<std::string, uint32_t>>& images) {
    std::string json = "{\"images\":{";
    for (size_t i = 0; i < images.size(); i++) {
        char crc[12];
        snprintf(crc, sizeof(crc), "%08X", unsigned(images[i].second));
        json += std::string(i ? "," : "") + "\"" + images[i].first + "\":\"" + crc + "\"";
    }
    json += std::string("},\"schema\":1,\"name\":\"") + name + "\",\"faces\":[{\"widgets\":[";
    for (size_t i = 0; i < images.size(); i++) {
        json += std::string(i ? "," : "") + "{\"type\":\"image\",\"image\":\"" + images[i].first + "\"}";
    }
    json += "]}]}";
    return Bytes(json.begin(), json.end());
}

// The hub's library of images and which go with whose config.
void giveImages(World& w, const Mac& mac, const std::vector<Bytes>& images) {
    std::vector<uint32_t>& want = w.hub.wantImages[mac];
    want.clear();
    for (const Bytes& b : images) {
        w.hub.library[crcOf(b)] = b;
        want.push_back(crcOf(b));
    }
}

}  // namespace

void test_images_go_before_the_config() {
    World w;
    const Bytes logo = makeImage(3000, 1), carbon = makeImage(20000, 2);
    giveImages(w, w.g1.mac, {logo, carbon});
    const Bytes cfg = configWithImages("A", {{"logo", crcOf(logo)}, {"carbon", crcOf(carbon)}});
    w.hub.assignments[w.g1.mac] = cfg;
    w.run(5000);

    TEST_ASSERT_TRUE(w.g1.stored == cfg);
    TEST_ASSERT_EQUAL(2, w.g1.images.size());
    TEST_ASSERT_TRUE(w.g1.images[crcOf(logo)] == logo);
    TEST_ASSERT_TRUE(w.g1.images[crcOf(carbon)] == carbon);
    // Both images were stored before the config arrived.
    TEST_ASSERT_EQUAL(3, w.g1.storeLog.size());
    TEST_ASSERT_EQUAL_STRING("config", w.g1.storeLog[2].c_str());
    TEST_ASSERT_EQUAL(0, w.hub.openImages);  // every image opened was closed again

    const HubManager::Gauge* g = w.hub.byMac(w.g1.mac);
    TEST_ASSERT_EQUAL(int(HubManager::ImageState::Have), int(HubManager::imageState(*g, crcOf(logo))));
    TEST_ASSERT_EQUAL(int(HubManager::ImageState::Have), int(HubManager::imageState(*g, crcOf(carbon))));
    TEST_ASSERT_TRUE(w.g2.images.empty());
}

void test_images_cross_the_bus_once() {
    World w;
    const Bytes logo = makeImage(9000, 3);
    giveImages(w, w.g1.mac, {logo});
    w.hub.assignments[w.g1.mac] = configWithImages("A", {{"logo", crcOf(logo)}});
    w.run(4000);
    TEST_ASSERT_EQUAL(1, w.g1.imageStores);

    // An edit to the config sends the config, not the image again.
    w.hub.assignments[w.g1.mac] = configWithImages("A edited", {{"logo", crcOf(logo)}});
    w.run(4000);
    TEST_ASSERT_EQUAL(2, w.g1.storeCount);
    TEST_ASSERT_EQUAL(1, w.g1.imageStores);

    // A power cycle, and a hub restart: the hub asks, the gauge has it.
    w.g1.boot();
    w.run(4000);
    w.hub.boot();
    w.run(4000);
    TEST_ASSERT_EQUAL(1, w.g1.imageStores);
    TEST_ASSERT_GREATER_THAN(0, w.g1.imageQueries);
    TEST_ASSERT_EQUAL(int(HubManager::ImageState::Have),
                      int(HubManager::imageState(*w.hub.byMac(w.g1.mac), crcOf(logo))));

    // A replacement gauge with empty storage gets it again.
    w.g1.images.clear();
    w.g1.boot();
    w.run(4000);
    TEST_ASSERT_EQUAL(2, w.g1.imageStores);
    TEST_ASSERT_EQUAL(1, w.g1.images.size());
}

void test_changed_image_replaces_the_old_one() {
    World w;
    const Bytes v1 = makeImage(5000, 4), v2 = makeImage(5000, 5);
    giveImages(w, w.g1.mac, {v1});
    w.hub.assignments[w.g1.mac] = configWithImages("A", {{"logo", crcOf(v1)}});
    w.run(4000);
    TEST_ASSERT_EQUAL(1, w.g1.images.count(crcOf(v1)));

    // The same name with new content is a new CRC: the config changes with it.
    giveImages(w, w.g1.mac, {v2});
    w.hub.assignments[w.g1.mac] = configWithImages("A", {{"logo", crcOf(v2)}});
    w.run(4000);
    TEST_ASSERT_EQUAL(1, w.g1.images.size());  // the gauge let the old one go when the config came
    TEST_ASSERT_TRUE(w.g1.images[crcOf(v2)] == v2);
    const HubManager::Gauge* g = w.hub.byMac(w.g1.mac);
    TEST_ASSERT_EQUAL(int(HubManager::ImageState::Unknown), int(HubManager::imageState(*g, crcOf(v1))));

    // Going back to the first version sends it again, since the gauge deleted it.
    giveImages(w, w.g1.mac, {v1});
    w.hub.assignments[w.g1.mac] = configWithImages("A", {{"logo", crcOf(v1)}});
    w.run(4000);
    TEST_ASSERT_TRUE(w.g1.images[crcOf(v1)] == v1);
    TEST_ASSERT_EQUAL(3, w.g1.imageStores);
}

void test_gauge_without_images_still_gets_its_config() {
    World w;
    w.g1.imagesSupported = false;  // firmware without images: says no, then refuses the kind
    w.g1.boot();
    const Bytes logo = makeImage(4000, 6);
    giveImages(w, w.g1.mac, {logo});
    const Bytes cfg = configWithImages("A", {{"logo", crcOf(logo)}});
    w.hub.assignments[w.g1.mac] = cfg;
    w.run(6000);
    TEST_ASSERT_TRUE(w.g1.stored == cfg);
    TEST_ASSERT_TRUE(w.g1.images.empty());
    TEST_ASSERT_EQUAL(int(HubManager::ImageState::Refused),
                      int(HubManager::imageState(*w.hub.byMac(w.g1.mac), crcOf(logo))));
    // Offered once, refused, not offered again.
    const int offered = w.g1.transfersOffered;
    w.run(20000);
    TEST_ASSERT_EQUAL(offered, w.g1.transfersOffered);
}

void test_image_gone_from_the_hub_is_skipped() {
    World w;
    const Bytes logo = makeImage(4000, 7);
    giveImages(w, w.g1.mac, {logo});
    w.hub.library.clear();  // deleted after the config was read
    const Bytes cfg = configWithImages("A", {{"logo", crcOf(logo)}});
    w.hub.assignments[w.g1.mac] = cfg;
    w.run(5000);
    TEST_ASSERT_TRUE(w.g1.stored == cfg);
    TEST_ASSERT_TRUE(w.g1.images.empty());
}

void test_try_sends_its_images_first() {
    World w;
    w.run(1500);
    const Bytes logo = makeImage(12000, 8);
    w.hub.library[crcOf(logo)] = logo;
    const Bytes t = configWithImages("try", {{"logo", crcOf(logo)}});
    const uint32_t crcs[] = {crcOf(logo)};
    const uint32_t node = w.g1.node.node();
    TEST_ASSERT_TRUE(w.hub.mgr.tryConfig(node, t.data(), uint32_t(t.size()), w.now, crcs, 1));
    TEST_ASSERT_TRUE(w.hub.mgr.pushBusy());
    TEST_ASSERT_FALSE(w.hub.mgr.tryConfig(node, t.data(), uint32_t(t.size()), w.now, crcs, 1));  // one at a time
    w.run(3000);
    TEST_ASSERT_TRUE(w.g1.trying);
    TEST_ASSERT_TRUE(w.g1.tryConfig == t);
    TEST_ASSERT_TRUE(w.g1.images[crcOf(logo)] == logo);
    TEST_ASSERT_EQUAL_STRING("image", w.g1.storeLog[0].c_str());
    TEST_ASSERT_FALSE(w.hub.mgr.pushBusy());
}

void test_big_images_go_over_wifi() {
    World w;
    w.hub.wifiOffered = true;
    const Bytes big = makeImage(40000, 7), small = makeImage(2000, 8);
    giveImages(w, w.g1.mac, {big, small});
    const Bytes cfg = configWithImages("A", {{"bg", crcOf(big)}, {"icon", crcOf(small)}});
    w.hub.assignments[w.g1.mac] = cfg;
    w.run(6000);
    TEST_ASSERT_TRUE(w.g1.stored == cfg);
    TEST_ASSERT_EQUAL(1, w.hub.wifiJoins);
    TEST_ASSERT_EQUAL(1, w.g1.wifiJoins);
    TEST_ASSERT_EQUAL(2, w.g1.images.size());
    // Both came over Wi-Fi in the one job, none over the bus, and the config followed.
    int wifi = 0, bus = 0;
    for (const std::string& s : w.g1.storeLog) {
        if (s == "wifi-image") wifi++;
        if (s == "image") bus++;
    }
    TEST_ASSERT_EQUAL(2, wifi);
    TEST_ASSERT_EQUAL(0, bus);
    TEST_ASSERT_EQUAL_STRING("config", w.g1.storeLog.back().c_str());
    TEST_ASSERT_EQUAL(0, w.hub.openImages);

    // Small images alone stay on the bus.
    const Bytes tiny = makeImage(1000, 9);
    giveImages(w, w.g2.mac, {tiny});
    w.hub.assignments[w.g2.mac] = configWithImages("B", {{"dot", crcOf(tiny)}});
    w.run(4000);
    TEST_ASSERT_EQUAL(1, w.hub.wifiJoins);
    TEST_ASSERT_EQUAL(1, w.g2.images.size());
    TEST_ASSERT_EQUAL_STRING("image", w.g2.storeLog[0].c_str());
}

void test_wifi_falls_back_to_the_bus() {
    World w;
    w.hub.wifiOffered = true;
    w.g1.wifiWorks = false;  // takes the join, reaches nothing
    const Bytes big = makeImage(40000, 11);
    giveImages(w, w.g1.mac, {big});
    const Bytes cfg = configWithImages("A", {{"bg", crcOf(big)}});
    w.hub.assignments[w.g1.mac] = cfg;
    w.run(2500);
    TEST_ASSERT_EQUAL(1, w.hub.wifiJoins);
    TEST_ASSERT_TRUE(w.g1.images.empty());  // the hub is giving Wi-Fi its chance
    w.run(12000);                            // the wait runs out: asked again, sent over the bus
    TEST_ASSERT_EQUAL(1, w.hub.wifiJoins);  // not offered twice
    TEST_ASSERT_TRUE(w.g1.stored == cfg);
    TEST_ASSERT_EQUAL(1, w.g1.images.size());
    TEST_ASSERT_EQUAL_STRING("image", w.g1.storeLog[0].c_str());

    // Firmware without Wi-Fi refuses the join: the bus at once.
    World w2;
    w2.hub.wifiOffered = true;
    w2.g1.wifiSupported = false;
    giveImages(w2, w2.g1.mac, {big});
    w2.hub.assignments[w2.g1.mac] = cfg;
    w2.run(5000);
    TEST_ASSERT_EQUAL(1, w2.hub.wifiJoins);
    TEST_ASSERT_TRUE(w2.g1.stored == cfg);
    TEST_ASSERT_EQUAL_STRING("image", w2.g1.storeLog[0].c_str());
}

void test_images_converge_on_a_lossy_bus() {
    World w;
    w.bus.dropPermille = 20;
    const Bytes a = makeImage(30000, 9), b = makeImage(15000, 10);
    giveImages(w, w.g1.mac, {a, b});
    giveImages(w, w.g2.mac, {b});
    w.hub.assignments[w.g1.mac] = configWithImages("one", {{"a", crcOf(a)}, {"b", crcOf(b)}});
    w.hub.assignments[w.g2.mac] = configWithImages("two", {{"b", crcOf(b)}});
    w.run(30000);
    TEST_ASSERT_TRUE(w.g1.images[crcOf(a)] == a);
    TEST_ASSERT_TRUE(w.g1.images[crcOf(b)] == b);
    TEST_ASSERT_TRUE(w.g2.images[crcOf(b)] == b);
    TEST_ASSERT_TRUE(w.g1.stored == w.hub.assignments[w.g1.mac]);
    TEST_ASSERT_TRUE(w.g2.stored == w.hub.assignments[w.g2.mac]);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_gauges_are_silent_without_a_hub);
    RUN_TEST(test_gauges_go_silent_when_the_hub_disappears);
    RUN_TEST(test_discovery);
    RUN_TEST(test_assigned_config_is_pushed_once);
    RUN_TEST(test_edited_config_is_pushed_again);
    RUN_TEST(test_gauge_reset_keeps_or_regains_its_config);
    RUN_TEST(test_hub_restart_rebuilds_roster_without_pushing);
    RUN_TEST(test_unplugged_gauge_goes_offline_and_returns);
    RUN_TEST(test_unplug_during_push_recovers);
    RUN_TEST(test_duplicate_node_ids_are_resolved);
    RUN_TEST(test_commands_reach_the_gauge_and_are_acknowledged);
    RUN_TEST(test_alert_message_stays_up_only_while_the_hub_repeats_it);
    RUN_TEST(test_alert_rules_are_carried_out_on_the_gauges);
    RUN_TEST(test_alert_switches_a_gauge_to_another_config_and_back);
    RUN_TEST(test_alert_naming_a_missing_config_does_nothing);
    RUN_TEST(test_hub_time_reaches_the_gauges);
    RUN_TEST(test_intro_plays_across_the_gauges_in_order);
    RUN_TEST(test_intro_goes_after_a_wait_when_a_gauge_is_missing);
    RUN_TEST(test_higher_alert_takes_over_the_message);
    RUN_TEST(test_theme_colours_reach_every_gauge);
    RUN_TEST(test_try_config_is_shown_but_not_stored);
    RUN_TEST(test_refused_config_is_not_retried_until_the_gauge_changes);
    RUN_TEST(test_config_newer_than_gauge_firmware_is_not_sent);
    RUN_TEST(test_gauge_uploads_its_config_on_request);
    RUN_TEST(test_converges_on_a_lossy_bus);
    RUN_TEST(test_channel_store_staleness);
    RUN_TEST(test_images_go_before_the_config);
    RUN_TEST(test_images_cross_the_bus_once);
    RUN_TEST(test_changed_image_replaces_the_old_one);
    RUN_TEST(test_gauge_without_images_still_gets_its_config);
    RUN_TEST(test_image_gone_from_the_hub_is_skipped);
    RUN_TEST(test_try_sends_its_images_first);
    RUN_TEST(test_images_converge_on_a_lossy_bus);
    RUN_TEST(test_big_images_go_over_wifi);
    RUN_TEST(test_wifi_falls_back_to_the_bus);
    return UNITY_END();
}
