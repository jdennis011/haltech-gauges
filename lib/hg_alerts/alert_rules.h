#pragma once

// Alerts: rules the hub checks against the live data. While a rule's
// condition holds, the gauges show its message, switch to its face, or both.
// This is the rule format, its parser and the evaluation; the hub adds the
// storage and the sending. docs/alerts.md describes the format.

#include <stddef.h>
#include <stdint.h>

#include <ArduinoJson.h>
#include <string>
#include <vector>

#include "channel_store.h"
#include "units.h"

namespace hg {
namespace alerts {

constexpr size_t kMaxRules = 16;
constexpr size_t kMaxName = 24;
constexpr size_t kMaxMessage = 32;  // bytes; what one alert message carries on the bus
constexpr float kMaxSeconds = 600;
constexpr int kMaxFace = 7;         // face index, from 0

enum class When : uint8_t {
    Above,    // value > `value`
    Below,    // value < `value`
    Outside,  // value < `low` or > `high`
    Inside,   // `low` <= value <= `high`
    On,       // a status flag is set
    Off,      // a status flag is clear
    Changes,  // the value is different from the last reading
};

struct Rule {
    std::string name;
    bool enabled = true;
    int16_t channel = -1;
    const DisplayUnit* unit = nullptr;  // the unit `value`, `low` and `high` are in
    When when = When::Above;
    float value = 0;
    float low = 0;
    float high = 0;
    uint32_t delayMs = 0;     // the condition must hold this long before the alert fires
    uint32_t holdMs = 3000;   // and the alert stays this long after it stops holding

    std::string message;      // shown over the face; empty for none
    uint32_t color = 0xFF3B30;
    int16_t face = -1;        // face to switch to; -1 leaves the face alone
    bool restore = true;      // go back to the earlier face when the alert ends
    bool allGauges = true;
    uint8_t mac[6] = {};      // the one gauge it applies to, when not all
};

struct ParseResult {
    bool ok = false;
    std::string error;  // names the rule, e.g. "alert 2 (Coolant hot): 'value' is required"
};

// Parses a JSON list of rules. `out` is only changed on success.
ParseResult parseRules(JsonArrayConst list, std::vector<Rule>& out);
ParseResult parseRules(const uint8_t* data, size_t len, std::vector<Rule>& out);
// Writes a rule back out in the form parseRules() reads.
void toJson(const Rule& rule, JsonObject out);

struct State {
    bool active = false;     // the alert is in force
    bool haveValue = false;
    float value = 0;         // the last reading, in the rule's unit
    uint32_t count = 0;      // times it has fired since the rules were set

    // Evaluation bookkeeping.
    bool pending = false;    // the condition holds but not yet for `delayMs`
    uint32_t sinceMs = 0;    // when it started holding
    uint32_t lastTrueMs = 0; // when it last held
    bool haveLast = false;
    float last = 0;          // previous reading, for When::Changes
    bool testing = false;
    uint32_t testUntilMs = 0;
};

class Engine {
public:
    // Replaces the rules; every alert starts out inactive.
    void setRules(std::vector<Rule> rules);
    const std::vector<Rule>& rules() const { return rules_; }
    const State& state(size_t index) const { return states_[index]; }

    // Checks every rule against the live data. True if an alert fired or ended.
    bool evaluate(const ChannelStore& store, uint32_t nowMs);

    // Puts a rule in force for a while whatever its condition says, to see
    // what it does. False if there is no such rule.
    bool test(size_t index, uint32_t nowMs, uint32_t forMs);

    // The rule whose message this gauge should show, or whose face it should
    // be on: the first active one in list order that applies to it. -1 for none.
    int messageFor(const uint8_t mac[6]) const;
    int faceFor(const uint8_t mac[6]) const;

private:
    bool appliesTo(const Rule& rule, const uint8_t mac[6]) const;

    std::vector<Rule> rules_;
    std::vector<State> states_;
};

}  // namespace alerts
}  // namespace hg
