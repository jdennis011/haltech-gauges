// End-to-end test of the management protocol: a hub and several gauges on a
// simulated CAN bus, exercising discovery, config reconciliation, resets,
// unplugging, duplicate ids and commands.

#include <string.h>
#include <unity.h>

#include <array>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "alert_dispatch.h"
#include "alert_rules.h"
#include "channel_store.h"
#include "crc32.h"
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
    uint8_t activeFace = 0;
    int identifyCount = 0;
    int storeCount = 0;
    int transfersOffered = 0;
    int rejectCount = 0;
    bool rejectConfigs = false;
    bool alertShown = false;
    std::string alertText;
    int alertChanges = 0;

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
    static void onStatus(Status& s, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        s.configCrc = g->stored.empty() ? 0 : crc32(g->stored.data(), g->stored.size());
        s.activeFace = g->activeFace;
        s.faceCount = 3;
        s.vbusMillivolts = 4900;
        s.flags = g->trying ? kStatusFlagTrying : 0;
    }
    static XferStatus onAccept(XferKind, uint32_t size, uint8_t** buffer, void* ctx) {
        SimGauge* g = static_cast<SimGauge*>(ctx);
        g->transfersOffered++;
        if (size > 32 * 1024) return XferStatus::TooBig;
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
        if (kind == XferKind::TryConfig) {
            g->tryConfig.assign(data, data + size);
            g->trying = true;
        } else {
            g->stored.assign(data, data + size);
            g->storeCount++;
        }
        return true;
    }

    void boot(uint32_t nodeId) {
        trying = false;
        tryConfig.clear();
        activeFace = 0;
        port.rx.clear();
        GaugeNode::Handler h = {};
        h.command = onCommand;
        h.status = onStatus;
        h.transfer = {onAccept, onComplete, nullptr, this};
        h.alert = onAlert;
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
    uint8_t configSchema = 1;
    Bytes uploadBuffer;
    Bytes uploaded;
    int changes = 0;

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
        return true;
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
        h.ctx = this;
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
    RUN_TEST(test_higher_alert_takes_over_the_message);
    RUN_TEST(test_try_config_is_shown_but_not_stored);
    RUN_TEST(test_refused_config_is_not_retried_until_the_gauge_changes);
    RUN_TEST(test_config_newer_than_gauge_firmware_is_not_sent);
    RUN_TEST(test_gauge_uploads_its_config_on_request);
    RUN_TEST(test_converges_on_a_lossy_bus);
    RUN_TEST(test_channel_store_staleness);
    return UNITY_END();
}
