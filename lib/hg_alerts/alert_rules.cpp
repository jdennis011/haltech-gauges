#include "alert_rules.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "haltech.h"

namespace hg {
namespace alerts {

namespace {

struct WhenName {
    const char* name;
    When value;
};
const WhenName kWhen[] = {
    {"above", When::Above},   {"below", When::Below}, {"outside", When::Outside},
    {"inside", When::Inside}, {"on", When::On},       {"off", When::Off},
    {"changes", When::Changes},
};

const char* whenName(When w) {
    for (const WhenName& n : kWhen) {
        if (n.value == w) return n.name;
    }
    return "above";
}

int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = char(tolower(uint8_t(c)));
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

// "#RGB" or "#RRGGBB".
bool parseColor(const char* s, uint32_t& out) {
    if (!s || s[0] != '#') return false;
    const size_t n = strlen(s + 1);
    if (n != 3 && n != 6) return false;
    uint32_t v = 0;
    for (size_t i = 0; i < n; i++) {
        const int d = hexDigit(s[1 + i]);
        if (d < 0) return false;
        v = (v << 4) | uint32_t(d);
        if (n == 3) v = (v << 4) | uint32_t(d);
    }
    out = v;
    return true;
}

// "AA:BB:CC:DD:EE:FF".
bool parseMac(const char* s, uint8_t mac[6]) {
    if (!s || strlen(s) != 17) return false;
    for (int i = 0; i < 6; i++) {
        const int hi = hexDigit(s[i * 3]);
        const int lo = hexDigit(s[i * 3 + 1]);
        if (hi < 0 || lo < 0) return false;
        if (i < 5 && s[i * 3 + 2] != ':') return false;
        mac[i] = uint8_t((hi << 4) | lo);
    }
    return true;
}

bool readNumber(JsonObjectConst o, const char* key, float& out, std::string& why) {
    JsonVariantConst v = o[key];
    if (!v.is<float>()) {
        why = std::string("'") + key + "' must be a number";
        return false;
    }
    out = v.as<float>();
    return true;
}

bool readSeconds(JsonObjectConst o, const char* key, float fallback, uint32_t& outMs, std::string& why) {
    float s = fallback;
    if (!o[key].isNull() && !readNumber(o, key, s, why)) return false;
    if (!(s >= 0 && s <= kMaxSeconds)) {
        why = std::string("'") + key + "' must be between 0 and 600 seconds";
        return false;
    }
    outMs = uint32_t(s * 1000 + 0.5f);
    return true;
}

bool parseRule(JsonObjectConst o, Rule& r, std::string& why) {
    if (o.isNull()) {
        why = "must be an object";
        return false;
    }
    const char* name = o["name"] | "";
    if (strlen(name) > kMaxName) {
        why = "'name' is longer than 24 characters";
        return false;
    }
    r.name = name;
    r.enabled = o["enabled"] | true;

    const char* channel = o["channel"].as<const char*>();
    if (!channel) {
        why = "'channel' is required";
        return false;
    }
    const int id = findChannel(channel);
    if (id < 0) {
        why = std::string("unknown channel '") + channel + "'";
        return false;
    }
    r.channel = int16_t(id);
    const char* unit = o["unit"].isNull() ? nullptr : o["unit"].as<const char*>();
    r.unit = findDisplayUnit(channelDef(ChannelId(id)).unit, unit);
    if (!r.unit) {
        why = std::string("unit '") + (unit ? unit : "") + "' is not available for channel '" + channel + "'";
        return false;
    }

    const char* when = o["when"].as<const char*>();
    bool known = false;
    for (const WhenName& n : kWhen) {
        if (when && strcmp(when, n.name) == 0) {
            r.when = n.value;
            known = true;
        }
    }
    if (!known) {
        why = "'when' must be one of above, below, outside, inside, on, off, changes";
        return false;
    }
    if (r.when == When::Above || r.when == When::Below) {
        if (!readNumber(o, "value", r.value, why)) return false;
    } else if (r.when == When::Outside || r.when == When::Inside) {
        if (!readNumber(o, "low", r.low, why) || !readNumber(o, "high", r.high, why)) return false;
        if (!(r.low < r.high)) {
            why = "'low' must be less than 'high'";
            return false;
        }
    }
    if (!readSeconds(o, "for", 0, r.delayMs, why)) return false;
    if (!readSeconds(o, "hold", 3, r.holdMs, why)) return false;

    const char* message = o["message"] | "";
    if (strlen(message) > kMaxMessage) {
        why = "'message' is longer than 32 characters";
        return false;
    }
    r.message = message;
    if (!o["color"].isNull() && !parseColor(o["color"].as<const char*>(), r.color)) {
        why = "'color' must look like #FF3B30";
        return false;
    }

    r.face = -1;
    if (!o["face"].isNull()) {
        if (!o["face"].is<int>() || o["face"].as<int>() < 0 || o["face"].as<int>() > kMaxFace) {
            why = "'face' must be a face number from 0 to 7";
            return false;
        }
        r.face = int16_t(o["face"].as<int>());
    }
    r.restore = o["restore"] | true;
    if (r.message.empty() && r.face < 0) {
        why = "needs a 'message' to show or a 'face' to switch to";
        return false;
    }

    const char* gauge = o["gauge"] | "all";
    r.allGauges = strcmp(gauge, "all") == 0 || gauge[0] == 0;
    if (!r.allGauges && !parseMac(gauge, r.mac)) {
        why = "'gauge' must be \"all\" or a MAC like AA:BB:CC:DD:EE:FF";
        return false;
    }
    return true;
}

bool conditionHolds(const Rule& r, float v) {
    switch (r.when) {
        case When::Above: return v > r.value;
        case When::Below: return v < r.value;
        case When::Outside: return v < r.low || v > r.high;
        case When::Inside: return v >= r.low && v <= r.high;
        case When::On: return v > 0.5f;
        case When::Off: return v <= 0.5f;
        case When::Changes: return false;  // an event, handled by the caller
    }
    return false;
}

}  // namespace

ParseResult parseRules(JsonArrayConst list, std::vector<Rule>& out) {
    ParseResult result;
    if (list.isNull()) {
        result.error = "alerts must be a list";
        return result;
    }
    if (list.size() > kMaxRules) {
        result.error = "more than 16 alerts";
        return result;
    }
    std::vector<Rule> rules;
    rules.reserve(list.size());
    size_t index = 0;
    for (JsonVariantConst item : list) {
        index++;
        Rule r;
        std::string why;
        if (!parseRule(item.as<JsonObjectConst>(), r, why)) {
            char head[48];
            const char* name = item["name"] | "";
            snprintf(head, sizeof(head), name[0] ? "alert %u (%.24s): " : "alert %u: ", unsigned(index), name);
            result.error = head + why;
            return result;
        }
        rules.push_back(std::move(r));
    }
    out = std::move(rules);
    result.ok = true;
    return result;
}

ParseResult parseRules(const uint8_t* data, size_t len, std::vector<Rule>& out) {
    JsonDocument doc;
    if (deserializeJson(doc, reinterpret_cast<const char*>(data), len)) {
        ParseResult result;
        result.error = "not valid JSON";
        return result;
    }
    // Either the bare list or {"rules": [...]}.
    JsonVariantConst root = doc.as<JsonVariantConst>();
    if (root.is<JsonObjectConst>()) return parseRules(root["rules"].as<JsonArrayConst>(), out);
    return parseRules(root.as<JsonArrayConst>(), out);
}

void toJson(const Rule& r, JsonObject o) {
    const ChannelDef& def = channelDef(ChannelId(r.channel));
    o["name"] = r.name;
    o["enabled"] = r.enabled;
    o["channel"] = def.name;
    if (r.unit && r.unit->name[0]) o["unit"] = r.unit->name;
    o["when"] = whenName(r.when);
    if (r.when == When::Above || r.when == When::Below) {
        o["value"] = r.value;
    } else if (r.when == When::Outside || r.when == When::Inside) {
        o["low"] = r.low;
        o["high"] = r.high;
    }
    o["for"] = r.delayMs / 1000.0f;
    o["hold"] = r.holdMs / 1000.0f;
    if (!r.message.empty()) o["message"] = r.message;
    char buf[18];
    snprintf(buf, sizeof(buf), "#%06lX", (unsigned long)(r.color & 0xFFFFFF));
    o["color"] = buf;
    if (r.face >= 0) {
        o["face"] = r.face;
        o["restore"] = r.restore;
    }
    if (r.allGauges) {
        o["gauge"] = "all";
    } else {
        snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", r.mac[0], r.mac[1], r.mac[2], r.mac[3],
                 r.mac[4], r.mac[5]);
        o["gauge"] = buf;
    }
}

void Engine::setRules(std::vector<Rule> rules) {
    rules_ = std::move(rules);
    states_.assign(rules_.size(), State());
}

bool Engine::evaluate(const ChannelStore& store, uint32_t nowMs) {
    bool changed = false;
    for (size_t i = 0; i < rules_.size(); i++) {
        const Rule& r = rules_[i];
        State& s = states_[i];
        const bool was = s.active;

        float v = 0;
        s.haveValue = store.get(ChannelId(r.channel), nowMs, v);
        if (s.haveValue) {
            v = toDisplay(*r.unit, v);
            s.value = v;
        }

        if (!r.enabled) {
            s.active = s.pending = s.haveLast = false;
        } else if (r.when == When::Changes) {
            // Each change puts the alert in force for the hold time.
            if (s.haveValue && s.haveLast && fabsf(v - s.last) > 1e-6f) {
                if (!s.active) s.count++;
                s.active = true;
                s.lastTrueMs = nowMs;
            } else if (s.active && uint32_t(nowMs - s.lastTrueMs) >= r.holdMs) {
                s.active = false;
            }
            // A channel that went quiet does not count as changed when it comes back.
            s.haveLast = s.haveValue;
            s.last = v;
        } else if (s.haveValue && conditionHolds(r, v)) {
            if (!s.pending) {
                s.pending = true;
                s.sinceMs = nowMs;
            }
            if (uint32_t(nowMs - s.sinceMs) >= r.delayMs) {
                if (!s.active) s.count++;
                s.active = true;
                s.lastTrueMs = nowMs;
            } else if (s.active && uint32_t(nowMs - s.lastTrueMs) >= r.holdMs) {
                s.active = false;
            }
        } else {
            s.pending = false;
            if (s.active && uint32_t(nowMs - s.lastTrueMs) >= r.holdMs) s.active = false;
        }

        if (s.testing) {
            if (int32_t(s.testUntilMs - nowMs) > 0) {
                s.active = true;
                // When the test ends the alert goes with it unless its condition holds.
                if (!s.pending || uint32_t(nowMs - s.sinceMs) < r.delayMs) s.lastTrueMs = nowMs - r.holdMs;
            } else {
                s.testing = false;
            }
        }
        if (s.active != was) changed = true;
    }
    return changed;
}

bool Engine::test(size_t index, uint32_t nowMs, uint32_t forMs) {
    if (index >= states_.size()) return false;
    states_[index].testing = true;
    states_[index].testUntilMs = nowMs + forMs;
    return true;
}

bool Engine::appliesTo(const Rule& rule, const uint8_t mac[6]) const {
    return rule.allGauges || memcmp(rule.mac, mac, 6) == 0;
}

int Engine::messageFor(const uint8_t mac[6]) const {
    for (size_t i = 0; i < rules_.size(); i++) {
        if (states_[i].active && !rules_[i].message.empty() && appliesTo(rules_[i], mac)) return int(i);
    }
    return -1;
}

int Engine::faceFor(const uint8_t mac[6]) const {
    for (size_t i = 0; i < rules_.size(); i++) {
        if (states_[i].active && rules_[i].face >= 0 && appliesTo(rules_[i], mac)) return int(i);
    }
    return -1;
}

}  // namespace alerts
}  // namespace hg
