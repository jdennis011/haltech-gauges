#include "hub_manager.h"

#include <string.h>

namespace hg {
namespace proto {

namespace {

// How long to leave a gauge alone after a push failed for a transient reason.
const uint32_t kPushRetryMs = 3000;
// Two hellos with different MACs this close together mean two gauges share an id.
const uint32_t kDuplicateWindowMs = 5000;

// True once `now` has reached `deadline`, across the 32-bit wrap.
bool reached(uint32_t now, uint32_t deadline) { return int32_t(now - deadline) >= 0; }

}  // namespace

void HubManager::init(const XferLink& link, const Handler& handler, uint32_t nowMs) {
    link_ = link;
    handler_ = handler;
    for (Gauge& g : gauges_) g = Gauge();
    startMs_ = nowMs;
    beaconSent_ = false;
    pushActive_ = false;
    helloRequestPending_ = false;
    uploadArmed_ = false;
    sender_ = XferSender();
    uploadReceiver_ = XferReceiver();
}

void HubManager::notify() {
    if (handler_.changed) handler_.changed(handler_.ctx);
}

const HubManager::Gauge* HubManager::find(uint32_t node) const {
    for (const Gauge& g : gauges_) {
        if (g.used && g.node == node) return &g;
    }
    return nullptr;
}

HubManager::Gauge* HubManager::findMutable(uint32_t node) {
    return const_cast<Gauge*>(find(node));
}

HubManager::Gauge* HubManager::findOrAdd(uint32_t node, uint32_t nowMs) {
    if (Gauge* g = findMutable(node)) return g;

    // Prefer a free slot, otherwise reuse the gauge that has been offline longest.
    Gauge* slot = nullptr;
    for (Gauge& g : gauges_) {
        if (!g.used) {
            slot = &g;
            break;
        }
        if (!g.online && (!slot || uint32_t(nowMs - g.lastSeenMs) > uint32_t(nowMs - slot->lastSeenMs))) {
            slot = &g;
        }
    }
    if (!slot) return nullptr;
    *slot = Gauge();
    slot->used = true;
    slot->node = node;
    slot->nextPushMs = nowMs;
    return slot;
}

bool HubManager::anyOnline() const {
    for (const Gauge& g : gauges_) {
        if (g.used && g.online) return true;
    }
    return false;
}

bool HubManager::command(uint32_t node, Cmd cmd, uint8_t arg) {
    Gauge* g = findMutable(node);
    if (!g || !g->online) return false;
    Command c;
    c.cmd = cmd;
    c.arg = arg;
    if (!send(pack(c, node))) return false;
    g->haveAck = false;
    return true;
}

bool HubManager::alert(uint32_t node, const Alert& alert, uint8_t seq) {
    const Gauge* g = find(node);
    if (!g || !g->online) return false;
    CanFrame frames[kAlertMaxFrames];
    const size_t count = packAlert(alert, seq, node, frames);
    for (size_t i = 0; i < count; i++) {
        if (!send(frames[i])) return false;
    }
    return true;
}

bool HubManager::themeColours(uint32_t node, const uint32_t colours[kThemeColourCount]) {
    if (node == kBroadcastNode) {
        if (!anyOnline()) return false;
    } else {
        const Gauge* g = find(node);
        if (!g || !g->online) return false;
    }
    CanFrame frames[kThemeColourFrames];
    const size_t count = packThemeColours(colours, node, frames);
    for (size_t i = 0; i < count; i++) {
        if (!send(frames[i])) return false;
    }
    return true;
}

bool HubManager::tryConfig(uint32_t node, const uint8_t* data, uint32_t size, uint32_t nowMs) {
    const Gauge* g = find(node);
    if (!g || !g->online || sender_.busy()) return false;
    pushActive_ = true;
    pushIsTry_ = true;
    sender_.start(link_, node, Dir::HubToGauge, ++session_, XferKind::TryConfig, data, size, nowMs);
    return true;
}

bool HubManager::requestUpload(uint32_t node, uint32_t nowMs) {
    const Gauge* g = find(node);
    (void)nowMs;
    if (!g || !g->online || uploadBusy()) return false;
    uploadReceiver_.init(link_, node, Dir::GaugeToHub, handler_.upload);
    uploadArmed_ = true;
    return command(node, Cmd::SendConfig, 0);
}

void HubManager::onFrame(const CanFrame& frame, uint32_t nowMs) {
    if (!frame.extended || idDir(frame.id) != Dir::GaugeToHub) return;
    const uint32_t node = idNode(frame.id);
    const Msg msg = idMsg(frame.id);

    Gauge* g = findOrAdd(node, nowMs);
    if (!g) return;  // roster full of online gauges
    g->lastSeenMs = nowMs;
    if (!g->online) {
        g->online = true;
        notify();
        if (handler_.online) handler_.online(*g, handler_.ctx);
    }

    switch (msg) {
        case Msg::Hello: {
            Hello h;
            if (!unpack(frame, h)) return;
            const bool macChanged = g->haveHello && memcmp(g->mac, h.mac, 6) != 0;
            g->duplicateId = macChanged && uint32_t(nowMs - g->helloMs) < kDuplicateWindowMs;
            memcpy(g->mac, h.mac, 6);
            g->protocolVersion = h.protocolVersion;
            g->schemaVersion = h.schemaVersion;
            g->haveHello = true;
            g->helloMs = nowMs;
            // A gauge that (re)announces itself may have new firmware; forget old refusals.
            g->pushFailed = false;
            g->nextPushMs = nowMs;
            notify();
            break;
        }
        case Msg::Info:
            if (unpack(frame, g->info)) {
                g->haveInfo = true;
                notify();
            }
            break;

        case Msg::Status: {
            Status s;
            if (!unpack(frame, s)) return;
            const bool changed = !g->haveStatus || s.configCrc != g->status.configCrc ||
                                 s.activeFace != g->status.activeFace ||
                                 s.faceCount != g->status.faceCount || s.flags != g->status.flags;
            g->status = s;
            g->haveStatus = true;
            // We restarted and this gauge is already running: ask who it is.
            if (!g->haveHello && !helloRequestPending_) {
                helloRequestPending_ = true;
                helloRequestNode_ = node;
            }
            if (changed) notify();
            break;
        }
        case Msg::Command: {
            Request rq;
            if (unpack(frame, rq) && handler_.request) handler_.request(*g, rq, handler_.ctx);
            break;
        }
        case Msg::CommandAck:
            if (unpack(frame, g->ack)) {
                g->haveAck = true;
                notify();
            }
            break;

        case Msg::XferAck:
            sender_.onFrame(frame, nowMs);
            break;

        case Msg::XferBegin:
        case Msg::XferData:
        case Msg::XferDataLast:
        case Msg::XferEnd:
        case Msg::XferAbort:
            if (uploadArmed_) uploadReceiver_.onFrame(frame, nowMs);
            break;

        default:
            break;
    }
}

void HubManager::finishPush(uint32_t nowMs) {
    pushActive_ = false;
    Gauge* g = findMutable(sender_.node());
    if (!g) return;
    const XferStatus result = sender_.result();

    if (pushIsTry_) {
        g->pushResult = result;
        notify();
        return;
    }

    g->pushResult = result;
    switch (result) {
        case XferStatus::Done:
            // The gauge stored it; its next status will confirm.
            g->status.configCrc = pushCrc_;
            g->pushFailed = false;
            break;
        case XferStatus::Rejected:
        case XferStatus::TooBig:
        case XferStatus::BadKind:
            // Sending the same bytes again would get the same answer.
            g->pushFailed = true;
            g->failedCrc = pushCrc_;
            break;
        default:
            g->nextPushMs = nowMs + kPushRetryMs;
            break;
    }
    notify();
}

void HubManager::reconcile(uint32_t nowMs) {
    for (Gauge& g : gauges_) {
        if (!g.used || !g.online || !g.haveHello || !g.haveStatus) continue;
        if (g.protocolVersion != kProtocolVersion) continue;
        if (g.status.flags & kStatusFlagTrying) continue;  // don't interrupt a preview
        if (!reached(nowMs, g.nextPushMs)) continue;

        Desired d;
        if (!handler_.desired(g, d, handler_.ctx)) continue;
        if (d.crc == g.status.configCrc) {
            g.pushFailed = false;
            continue;
        }
        if (g.pushFailed && g.failedCrc == d.crc) continue;
        if (d.schema > g.schemaVersion) {
            // The gauge's firmware is too old to understand this config.
            g.pushFailed = true;
            g.failedCrc = d.crc;
            g.pushResult = XferStatus::Rejected;
            notify();
            continue;
        }

        pushActive_ = true;
        pushIsTry_ = false;
        pushCrc_ = d.crc;
        sender_.start(link_, g.node, Dir::HubToGauge, ++session_, XferKind::Config, d.data, d.size,
                      nowMs);
        return;  // one transfer at a time
    }
}

void HubManager::poll(uint32_t nowMs) {
    if (!beaconSent_ || uint32_t(nowMs - lastBeaconMs_) >= kBeaconPeriodMs) {
        Beacon b;
        b.flags = beaconFlags_;
        b.uptimeSec = uint16_t(uint32_t(nowMs - startMs_) / 1000);
        if (send(pack(b))) {
            beaconSent_ = true;
            lastBeaconMs_ = nowMs;
        }
    }

    if (helloRequestPending_ && uint32_t(nowMs - lastHelloRequestMs_) >= 250) {
        if (send(packHelloRequest(helloRequestNode_))) {
            helloRequestPending_ = false;
            lastHelloRequestMs_ = nowMs;
        }
    }

    for (Gauge& g : gauges_) {
        if (g.used && g.online && uint32_t(nowMs - g.lastSeenMs) >= kGaugeTimeoutMs) {
            g.online = false;
            notify();
        }
    }

    sender_.poll(nowMs);
    if (uploadArmed_) uploadReceiver_.poll(nowMs);

    if (pushActive_ && !sender_.busy()) finishPush(nowMs);
    if (!sender_.busy() && !uploadBusy()) reconcile(nowMs);
}

}  // namespace proto
}  // namespace hg
