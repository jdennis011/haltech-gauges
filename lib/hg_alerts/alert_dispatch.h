#pragma once

// Carries out the alerts in force on the gauges in the hub's roster: puts each
// gauge's message up and keeps it up, takes it down, switches the config or
// the face and puts the earlier one back.

#include <stdint.h>

#include "alert_rules.h"
#include "hub_manager.h"

namespace hg {
namespace alerts {

class Dispatcher {
public:
    // How long a message outlives the hub's last repeat of it.
    static constexpr uint8_t kMessageTtlSeconds = 3;
    // A config switch that the gauge has not confirmed this long after the
    // transfer ended (its status comes once a second) is sent again...
    static constexpr uint32_t kLandTimeoutMs = 2500;
    // ...but never more often than this.
    static constexpr uint32_t kSwitchRetryMs = 3000;

    // Where the configs that alerts switch to come from.
    struct ConfigSource {
        // Fills `out` with the library config of this name as it goes to a
        // gauge (with its image map); false if there is no such config. The
        // data must stay valid until HubManager::pushBusy() is false.
        bool (*lookup)(const char* name, proto::HubManager::Desired& out, void* ctx);
        void* ctx;
    };
    void setConfigSource(const ConfigSource& source) { source_ = source; }

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

        // The config it was switched to.
        int config = -1;         // rule whose config it was sent; -1 when on its own
        bool inFlight = false;   // the transfer is still running
        bool landed = false;     // the gauge has said it is showing it
        bool faceSet = false;    // the rule's face was applied once it landed
        uint32_t switchMs = 0;   // when the transfer was handed to the manager
        uint32_t doneMs = 0;     // when it ended
        uint32_t landedMs = 0;
        uint32_t retryMs = 0;    // no new transfer before this
        bool restoreFace = false;  // put the saved face back once the switch ends
        bool restorePending = false;

        int face = -1;  // face it was switched to; -1 when left alone
        bool restore = false;
        bool haveSaved = false;  // the face it was on before any alert moved it
        uint8_t savedFace = 0;

        // Sounds, per rule: in force for this gauge at the last run (coming
        // into force plays it), a play owed, and when it last played.
        bool soundSeen[kMaxRules] = {};
        bool soundDue[kMaxRules] = {};
        uint32_t soundMs[kMaxRules] = {};
        bool soundFresh = true;  // alerts already in force when the slot starts do not play
        uint32_t soundGeneration = 0;
    };

    void serve(const Engine& engine, proto::HubManager& manager, const proto::HubManager::Gauge& gauge,
               Slot& slot, uint32_t nowMs);
    void serveSounds(const Engine& engine, proto::HubManager& manager, const proto::HubManager::Gauge& gauge,
                     Slot& slot, uint32_t nowMs);
    void serveConfig(const Rule& rule, int index, proto::HubManager& manager,
                     const proto::HubManager::Gauge& gauge, Slot& slot, uint32_t nowMs);
    void endConfig(proto::HubManager& manager, const proto::HubManager::Gauge& gauge, Slot& slot);

    Slot slots_[proto::HubManager::kMaxGauges];
    uint32_t generation_ = 0;
    ConfigSource source_ = {};
};

}  // namespace alerts
}  // namespace hg
