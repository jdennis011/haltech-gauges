#pragma once

// Carries out the alerts in force on the gauges in the hub's roster: puts each
// gauge's message up and keeps it up, takes it down, switches the face and
// puts the earlier one back.

#include <stdint.h>

#include "alert_rules.h"
#include "hub_manager.h"

namespace hg {
namespace alerts {

class Dispatcher {
public:
    // How long a message outlives the hub's last repeat of it.
    static constexpr uint8_t kMessageTtlSeconds = 3;

    // Sends whatever each online gauge needs. Call after Engine::evaluate(),
    // about ten times a second.
    void run(const Engine& engine, proto::HubManager& manager, uint32_t nowMs);

    // The rules were replaced: messages in force are sent afresh.
    void rulesChanged() { generation_++; }

private:
    // What a gauge in the roster has been told.
    struct Slot {
        uint32_t node = 0;
        int message = -1;  // rule whose message it is showing
        uint32_t messageGeneration = 0;
        uint8_t seq = 0;
        uint32_t sentMs = 0;
        int face = -1;  // face it was switched to; -1 when left alone
        bool restore = false;
        bool haveSaved = false;
        uint8_t savedFace = 0;
    };

    void serve(const Engine& engine, proto::HubManager& manager, const proto::HubManager::Gauge& gauge,
               Slot& slot, uint32_t nowMs);

    Slot slots_[proto::HubManager::kMaxGauges];
    uint32_t generation_ = 0;
};

}  // namespace alerts
}  // namespace hg
