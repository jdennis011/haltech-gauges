#include "alert_dispatch.h"

#include <string.h>

namespace hg {
namespace alerts {

using proto::HubManager;

namespace {

bool reached(uint32_t nowMs, uint32_t whenMs) { return int32_t(nowMs - whenMs) >= 0; }

// A status that arrived after `sinceMs`.
bool statusSince(const HubManager::Gauge& g, uint32_t sinceMs) {
    return g.haveStatus && int32_t(g.statusMs - sinceMs) > 0;
}

bool showingRamConfig(const HubManager::Gauge& g) {
    return (g.status.flags & proto::kStatusFlagTrying) != 0;
}

}  // namespace

void Dispatcher::run(const Engine& engine, HubManager& manager, uint32_t nowMs) {
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = manager.gauge(i);
        Slot& s = slots_[i];
        // A gauge that went away and came back has forgotten what it was
        // told. A steering wheel module shows nothing and gets no alerts.
        if (!g.used || !g.online || !g.haveHello || HubManager::isWheel(g)) {
            s = Slot();
            continue;
        }
        if (s.node != g.node) {
            s = Slot();
            s.node = g.node;
        }
        serve(engine, manager, g, s, nowMs);
    }
}

void Dispatcher::serve(const Engine& engine, HubManager& manager, const HubManager::Gauge& g, Slot& s,
                       uint32_t nowMs) {
    const std::vector<Rule>& rules = engine.rules();

    serveSounds(engine, manager, g, s, nowMs);

    // The message: sent when it changes, then repeated so the gauge keeps it up.
    const int message = engine.messageFor(g.mac);
    if (message >= 0) {
        const bool fresh = message != s.message || s.messageGeneration != generation_;
        if (fresh || uint32_t(nowMs - s.sentMs) >= proto::kAlertRepeatMs) {
            if (fresh) s.seq = uint8_t((s.seq + 1) & 0x0F);
            proto::Alert a;
            a.show = true;
            a.color = rules[message].color;
            a.ttlSeconds = kMessageTtlSeconds;
            strncpy(a.text, rules[message].message.c_str(), proto::kAlertMaxText);
            // A busy bus is tried again on the next run.
            if (manager.alert(g.node, a, s.seq)) {
                s.message = message;
                s.messageGeneration = generation_;
                s.sentMs = nowMs;
            }
        }
    } else if (s.message >= 0) {
        // A clear lost on the bus is covered by the gauge's own timeout.
        if (manager.alert(g.node, proto::Alert(), s.seq)) s.message = -1;
    }

    // The config: another config shown in place of the gauge's own while an
    // alert asks for it. Its face is the config rule's own, applied once the
    // config has landed; the face rules below wait until the switch ends.
    const int config = engine.configFor(g.mac);
    if (config >= 0) {
        serveConfig(rules[config], config, manager, g, s, nowMs);
        return;
    }
    if (s.config >= 0) {
        endConfig(manager, g, s);
        return;
    }
    if (s.restorePending) {
        // The switch has ended; the face it was on before comes back.
        if (!manager.command(g.node, proto::Cmd::SetFace, s.savedFace)) return;
        s.restorePending = false;
    }

    // The face: switched when an alert asks for one, put back when it ends.
    const int rule = engine.faceFor(g.mac);
    const int wanted = rule >= 0 ? rules[rule].face : -1;
    if (wanted == s.face) return;
    if (wanted >= 0) {
        if (s.face < 0 && !s.haveSaved && g.haveStatus) {
            s.savedFace = g.status.activeFace;
            s.haveSaved = true;
        }
        if (manager.command(g.node, proto::Cmd::SetFace, uint8_t(wanted))) {
            s.face = wanted;
            s.restore = rules[rule].restore;
        }
    } else if (!s.restore || !s.haveSaved || manager.command(g.node, proto::Cmd::SetFace, s.savedFace)) {
        s.face = -1;
        s.haveSaved = false;
    }
}

// A sound plays when its rule comes into force for the gauge, and again every
// `repeat` while it holds. It goes as a command, so a send lost to a busy bus
// is owed until it gets through. A gauge that has just appeared, or a new set
// of rules, starts from what is in force now without playing it.
void Dispatcher::serveSounds(const Engine& engine, HubManager& manager, const HubManager::Gauge& g, Slot& s,
                             uint32_t nowMs) {
    const std::vector<Rule>& rules = engine.rules();
    const size_t n = rules.size() < kMaxRules ? rules.size() : kMaxRules;
    if (s.soundFresh || s.soundGeneration != generation_) {
        for (size_t i = 0; i < kMaxRules; i++) {
            s.soundSeen[i] = i < n && engine.state(i).active;
            s.soundDue[i] = false;
            s.soundMs[i] = nowMs;
        }
        s.soundFresh = false;
        s.soundGeneration = generation_;
        return;
    }
    for (size_t i = 0; i < n; i++) {
        const Rule& r = rules[i];
        const bool active = engine.state(i).active && r.sound != proto::Sound::None && engine.applies(i, g.mac);
        if (active && !s.soundSeen[i]) s.soundDue[i] = true;
        s.soundSeen[i] = active;
        if (!active) {
            s.soundDue[i] = false;
            continue;
        }
        if (!s.soundDue[i] && r.repeatMs && uint32_t(nowMs - s.soundMs[i]) >= r.repeatMs) s.soundDue[i] = true;
        if (s.soundDue[i] && manager.command(g.node, proto::Cmd::PlaySound, uint8_t(r.sound))) {
            s.soundDue[i] = false;
            s.soundMs[i] = nowMs;
        }
    }
}

void Dispatcher::serveConfig(const Rule& rule, int index, HubManager& manager, const HubManager::Gauge& g,
                             Slot& s, uint32_t nowMs) {
    const bool busyForUs = manager.pushBusy() && manager.pushNode() == g.node;

    if (s.config == index) {
        if (!s.landed) {
            if (busyForUs) {
                s.inFlight = true;
            } else {
                if (s.inFlight) {
                    s.inFlight = false;
                    s.doneMs = nowMs;
                }
                if (statusSince(g, s.switchMs) && showingRamConfig(g)) {
                    // Landed: a status since the transfer went says it is showing a RAM config.
                    s.landed = true;
                    s.landedMs = nowMs;
                    s.faceSet = rule.face < 0;
                } else if (uint32_t(nowMs - s.doneMs) >= kLandTimeoutMs) {
                    // Refused, or lost on the bus: sent again after a pause.
                    s.config = -1;
                }
            }
        }
        if (s.landed) {
            if (!s.faceSet && manager.command(g.node, proto::Cmd::SetFace, uint8_t(rule.face))) s.faceSet = true;
            // Back on its own config (the page ended the switch, or a config
            // was stored over it) while the alert still holds: sent again.
            if (statusSince(g, s.landedMs) && !showingRamConfig(g) && reached(nowMs, s.retryMs)) s.config = -1;
        }
        if (s.config == index) return;
    }

    // Send it: a first switch, a different rule winning, or a second go.
    if (!reached(nowMs, s.retryMs) || manager.pushBusy()) return;
    if (s.config < 0 && !s.haveSaved && g.haveStatus) {
        // The face to go back to is the one before any alert moved it.
        s.savedFace = g.status.activeFace;
        s.haveSaved = true;
    }
    s.face = -1;  // the face rules start afresh once the switch ends
    s.retryMs = nowMs + kSwitchRetryMs;
    HubManager::Desired d;
    if (!source_.lookup || !source_.lookup(rule.config.c_str(), d, source_.ctx)) return;  // no such config
    if (!manager.tryConfig(g.node, d.data, d.size, nowMs, d.images, d.imageCount,
                           proto::XferKind::SwitchConfig)) {
        return;
    }
    s.config = index;
    s.inFlight = true;
    s.landed = false;
    s.faceSet = false;
    s.switchMs = nowMs;
    s.doneMs = nowMs;
    s.restoreFace = rule.restore;
    s.restorePending = false;
}

void Dispatcher::endConfig(HubManager& manager, const HubManager::Gauge& g, Slot& s) {
    // A switch still on its way would land after an "end" sent now; wait for it.
    if (manager.pushBusy() && manager.pushNode() == g.node) return;
    if (!manager.command(g.node, proto::Cmd::EndTry, 0)) return;
    s.config = -1;
    s.inFlight = false;
    s.landed = false;
    s.restorePending = s.restoreFace && s.haveSaved;
    if (!s.restorePending) s.haveSaved = false;
}

}  // namespace alerts
}  // namespace hg
