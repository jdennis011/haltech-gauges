#include "alert_dispatch.h"

#include <string.h>

namespace hg {
namespace alerts {

using proto::HubManager;

void Dispatcher::run(const Engine& engine, HubManager& manager, uint32_t nowMs) {
    for (size_t i = 0; i < HubManager::kMaxGauges; i++) {
        const HubManager::Gauge& g = manager.gauge(i);
        Slot& s = slots_[i];
        // A gauge that went away and came back has forgotten what it was told.
        if (!g.used || !g.online || !g.haveHello) {
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

    // The face: switched when an alert asks for one, put back when it ends.
    const int rule = engine.faceFor(g.mac);
    const int wanted = rule >= 0 ? rules[rule].face : -1;
    if (wanted == s.face) return;
    if (wanted >= 0) {
        if (s.face < 0 && g.haveStatus) {
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

}  // namespace alerts
}  // namespace hg
