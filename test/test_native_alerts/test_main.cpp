// Alert rules: parsing, and how they fire and end against live data.

#include <string.h>
#include <unity.h>

#include <string>
#include <vector>

#include "alert_rules.h"
#include "channel_store.h"

using namespace hg;
using namespace hg::alerts;

void setUp() {}
void tearDown() {}

namespace {

ParseResult parse(const std::string& json, std::vector<Rule>& out) {
    return parseRules(reinterpret_cast<const uint8_t*>(json.data()), json.size(), out);
}

void assertRejected(const std::string& json, const char* expectInError) {
    std::vector<Rule> rules;
    const ParseResult r = parse(json, rules);
    TEST_ASSERT_FALSE_MESSAGE(r.ok, json.c_str());
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(r.error.c_str(), expectInError), r.error.c_str());
}

// An engine fed by a channel store, stepped 100 ms at a time.
struct Rig {
    Engine engine;
    ChannelStore store;
    uint32_t now = 10000;
    int changes = 0;

    explicit Rig(const std::string& json) {
        std::vector<Rule> rules;
        const ParseResult r = parse(json, rules);
        TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
        engine.setRules(rules);
    }
    // Holds a channel at a value for a while, checking the rules as the hub does.
    void run(ChannelId id, float value, uint32_t ms) {
        for (uint32_t t = 0; t < ms; t += 100) {
            now += 100;
            store.set(id, value, true, now);
            if (engine.evaluate(store, now)) changes++;
        }
    }
    // Time passing with no data arriving.
    void silence(uint32_t ms) {
        for (uint32_t t = 0; t < ms; t += 100) {
            now += 100;
            if (engine.evaluate(store, now)) changes++;
        }
    }
    bool active(size_t i = 0) const { return engine.state(i).active; }
};

const uint8_t kMacA[6] = {0xA0, 0x76, 0x4E, 0x11, 0x22, 0x33};
const uint8_t kMacB[6] = {0xA0, 0x76, 0x4E, 0x44, 0x55, 0x66};

}  // namespace

void test_rules_parse_and_write_back() {
    const char* json = R"([
      {"name":"Coolant hot","channel":"coolant_temp","unit":"C","when":"above","value":105,"for":1.5,"hold":4,
       "message":"COOLANT HOT","color":"#FFCC00","face":2,"restore":false,"gauge":"A0:76:4E:11:22:33"},
      {"channel":"wideband_1","when":"outside","low":0.8,"high":1.1,"message":"LAMBDA"},
      {"name":"CEL","enabled":false,"channel":"check_engine_light","when":"on","face":0}
    ])";
    std::vector<Rule> rules;
    const ParseResult r = parse(json, rules);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    TEST_ASSERT_EQUAL(3, rules.size());

    TEST_ASSERT_EQUAL_STRING("Coolant hot", rules[0].name.c_str());
    TEST_ASSERT_EQUAL(int(CH_coolant_temp), rules[0].channel);
    TEST_ASSERT_EQUAL(int(When::Above), int(rules[0].when));
    TEST_ASSERT_EQUAL_FLOAT(105, rules[0].value);
    TEST_ASSERT_EQUAL(1500, rules[0].delayMs);
    TEST_ASSERT_EQUAL(4000, rules[0].holdMs);
    TEST_ASSERT_EQUAL_HEX32(0xFFCC00, rules[0].color);
    TEST_ASSERT_EQUAL(2, rules[0].face);
    TEST_ASSERT_FALSE(rules[0].restore);
    TEST_ASSERT_FALSE(rules[0].allGauges);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(kMacA, rules[0].mac, 6);

    // Defaults: enabled, every gauge, no delay, three seconds of hold, red, no face.
    TEST_ASSERT_TRUE(rules[1].enabled);
    TEST_ASSERT_TRUE(rules[1].allGauges);
    TEST_ASSERT_EQUAL(0, rules[1].delayMs);
    TEST_ASSERT_EQUAL(3000, rules[1].holdMs);
    TEST_ASSERT_EQUAL_HEX32(0xFF3B30, rules[1].color);
    TEST_ASSERT_EQUAL(-1, rules[1].face);
    TEST_ASSERT_FALSE(rules[2].enabled);
    TEST_ASSERT_EQUAL(0, rules[2].face);

    // What is written back reads as the same rules.
    JsonDocument doc;
    JsonArray list = doc.to<JsonArray>();
    for (const Rule& rule : rules) toJson(rule, list.add<JsonObject>());
    std::string text;
    serializeJson(doc, text);
    std::vector<Rule> again;
    TEST_ASSERT_TRUE(parse(text, again).ok);
    TEST_ASSERT_EQUAL(3, again.size());
    TEST_ASSERT_EQUAL_STRING("COOLANT HOT", again[0].message.c_str());
    TEST_ASSERT_EQUAL_FLOAT(105, again[0].value);
    TEST_ASSERT_EQUAL(1500, again[0].delayMs);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(kMacA, again[0].mac, 6);
    TEST_ASSERT_EQUAL_FLOAT(0.8f, again[1].low);
    TEST_ASSERT_EQUAL_FLOAT(1.1f, again[1].high);
    TEST_ASSERT_FALSE(again[2].enabled);

    // The page may send {"rules": [...]} as well as the bare list.
    TEST_ASSERT_TRUE(parse(R"({"rules":[{"channel":"rpm","when":"above","value":7000,"message":"SHIFT"}]})", again).ok);
    TEST_ASSERT_EQUAL(1, again.size());
    TEST_ASSERT_TRUE(parse("[]", again).ok);
    TEST_ASSERT_EQUAL(0, again.size());
}

void test_bad_rules_are_refused_and_named() {
    assertRejected(R"([{"channel":"rpm","when":"above","value":7000,"message":"OK"},
                       {"name":"Second","channel":"no_such","when":"on","message":"X"}])",
                   "alert 2 (Second): unknown channel 'no_such'");
    assertRejected(R"([{"when":"on","message":"X"}])", "alert 1: 'channel' is required");
    assertRejected(R"([{"channel":"rpm","when":"over","message":"X"}])", "'when' must be one of");
    assertRejected(R"([{"channel":"rpm","when":"above","message":"X"}])", "'value' must be a number");
    assertRejected(R"([{"channel":"rpm","when":"inside","low":5000,"high":4000,"message":"X"}])", "'low' must be less");
    assertRejected(R"([{"channel":"rpm","when":"above","value":1}])", "needs a 'message' to show or a 'face'");
    assertRejected(R"([{"channel":"rpm","when":"above","value":1,"face":8}])", "'face' must be");
    assertRejected(R"([{"channel":"rpm","when":"above","value":1,"message":"X","color":"red"}])", "'color' must look like");
    assertRejected(R"([{"channel":"rpm","when":"above","value":1,"message":"X","gauge":"left"}])", "'gauge' must be");
    assertRejected(R"([{"channel":"rpm","unit":"psi","when":"above","value":1,"message":"X"}])", "unit 'psi' is not available");
    assertRejected(R"([{"channel":"rpm","when":"above","value":1,"message":"X","for":601}])", "'for' must be between");
    assertRejected(R"([{"channel":"rpm","when":"above","value":1,"message":"123456789012345678901234567890123"}])",
                   "longer than 32");
    assertRejected("{\"rules\":5}", "alerts must be a list");
    assertRejected("nonsense", "not valid JSON");

    std::string many = "[";
    for (int i = 0; i < 17; i++) many += std::string(i ? "," : "") + R"({"channel":"rpm","when":"on","message":"X"})";
    assertRejected(many + "]", "more than 16");

    // A refused list leaves the caller's rules alone.
    std::vector<Rule> rules(2);
    TEST_ASSERT_FALSE(parse(R"([{"when":"on"}])", rules).ok);
    TEST_ASSERT_EQUAL(2, rules.size());
}

void test_above_fires_after_its_delay_and_holds() {
    Rig rig(R"([{"channel":"coolant_temp","when":"above","value":105,"for":1,"hold":2,"message":"HOT"}])");
    rig.run(CH_coolant_temp, 90, 1000);
    TEST_ASSERT_FALSE(rig.active());

    // Over the limit, but not yet for a second.
    rig.run(CH_coolant_temp, 106, 900);
    TEST_ASSERT_FALSE(rig.active());
    // A dip restarts the wait.
    rig.run(CH_coolant_temp, 104, 100);
    rig.run(CH_coolant_temp, 106, 900);
    TEST_ASSERT_FALSE(rig.active());
    rig.run(CH_coolant_temp, 106, 300);
    TEST_ASSERT_TRUE(rig.active());
    TEST_ASSERT_EQUAL(1, rig.engine.state(0).count);
    TEST_ASSERT_EQUAL_FLOAT(106, rig.engine.state(0).value);

    // Back under: it stays for the hold time, then ends.
    rig.run(CH_coolant_temp, 100, 1800);
    TEST_ASSERT_TRUE(rig.active());
    rig.run(CH_coolant_temp, 100, 400);
    TEST_ASSERT_FALSE(rig.active());
    TEST_ASSERT_EQUAL(2, rig.changes);  // fired once, ended once

    // The limit itself is not "above".
    rig.run(CH_coolant_temp, 105, 3000);
    TEST_ASSERT_FALSE(rig.active());
}

void test_thresholds_are_in_the_rules_unit() {
    // 221 F is 105 C.
    Rig rig(R"([{"channel":"coolant_temp","unit":"F","when":"above","value":221,"hold":0,"message":"HOT"}])");
    rig.run(CH_coolant_temp, 104, 300);
    TEST_ASSERT_FALSE(rig.active());
    rig.run(CH_coolant_temp, 106, 300);
    TEST_ASSERT_TRUE(rig.active());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 222.8f, rig.engine.state(0).value);
}

void test_range_conditions() {
    Rig rig(R"([{"channel":"wideband_1","when":"outside","low":0.8,"high":1.1,"hold":0,"message":"LAMBDA"},
                {"channel":"rpm","when":"inside","low":3000,"high":4000,"hold":0,"message":"BAND"},
                {"channel":"oil_pressure","when":"below","value":100,"hold":0,"message":"OIL"}])");
    rig.run(CH_wideband_1, 0.95f, 300);
    TEST_ASSERT_FALSE(rig.active(0));
    rig.run(CH_wideband_1, 1.2f, 300);
    TEST_ASSERT_TRUE(rig.active(0));
    rig.run(CH_wideband_1, 0.7f, 300);
    TEST_ASSERT_TRUE(rig.active(0));
    rig.run(CH_wideband_1, 1.0f, 300);
    TEST_ASSERT_FALSE(rig.active(0));

    rig.run(CH_rpm, 2999, 200);
    TEST_ASSERT_FALSE(rig.active(1));
    rig.run(CH_rpm, 3000, 200);
    TEST_ASSERT_TRUE(rig.active(1));
    rig.run(CH_rpm, 4001, 200);
    TEST_ASSERT_FALSE(rig.active(1));

    rig.run(CH_oil_pressure, 250, 200);
    TEST_ASSERT_FALSE(rig.active(2));
    rig.run(CH_oil_pressure, 60, 200);
    TEST_ASSERT_TRUE(rig.active(2));
}

void test_flags_and_changes() {
    Rig rig(R"([{"channel":"check_engine_light","when":"on","hold":0,"message":"CHECK ENGINE"},
                {"channel":"check_engine_light","when":"off","hold":0,"face":1},
                {"channel":"check_engine_light","when":"changes","hold":2,"message":"CEL CHANGED"}])");
    rig.run(CH_check_engine_light, 0, 500);
    TEST_ASSERT_FALSE(rig.active(0));
    TEST_ASSERT_TRUE(rig.active(1));
    TEST_ASSERT_FALSE(rig.active(2));  // the first reading is not a change

    rig.run(CH_check_engine_light, 1, 500);
    TEST_ASSERT_TRUE(rig.active(0));
    TEST_ASSERT_FALSE(rig.active(1));
    TEST_ASSERT_TRUE(rig.active(2));
    TEST_ASSERT_EQUAL(1, rig.engine.state(2).count);

    // A change shows for its hold time, then goes whether the flag is set or not.
    rig.run(CH_check_engine_light, 1, 1800);
    TEST_ASSERT_FALSE(rig.active(2));
    TEST_ASSERT_TRUE(rig.active(0));

    rig.run(CH_check_engine_light, 0, 300);
    TEST_ASSERT_TRUE(rig.active(2));
    TEST_ASSERT_EQUAL(2, rig.engine.state(2).count);
}

void test_missing_data_ends_an_alert_and_is_not_a_change() {
    Rig rig(R"([{"channel":"rpm","when":"above","value":7000,"hold":1,"message":"SHIFT"},
                {"channel":"rpm","when":"changes","hold":1,"message":"RPM MOVED"},
                {"channel":"rpm","when":"below","value":500,"hold":0,"message":"STALLED"}])");
    rig.run(CH_rpm, 7200, 300);
    TEST_ASSERT_TRUE(rig.active(0));
    TEST_ASSERT_FALSE(rig.engine.state(1).active);

    // The ECU goes quiet: rpm is stale after half a second, and the alert follows.
    rig.silence(3000);
    TEST_ASSERT_FALSE(rig.engine.state(0).haveValue);
    TEST_ASSERT_FALSE(rig.active(0));
    TEST_ASSERT_FALSE(rig.active(2));  // no reading is not a low reading

    // Data coming back at a different value is not reported as a change.
    rig.run(CH_rpm, 900, 300);
    TEST_ASSERT_FALSE(rig.active(1));
    rig.run(CH_rpm, 950, 100);
    TEST_ASSERT_TRUE(rig.active(1));
}

void test_disabled_rules_never_fire_and_test_forces_one() {
    Rig rig(R"([{"enabled":false,"channel":"rpm","when":"above","value":1000,"message":"A"},
                {"channel":"rpm","when":"above","value":9000,"hold":3,"message":"B"}])");
    rig.run(CH_rpm, 5000, 500);
    TEST_ASSERT_FALSE(rig.active(0));
    TEST_ASSERT_FALSE(rig.active(1));

    TEST_ASSERT_TRUE(rig.engine.test(1, rig.now, 2000));
    TEST_ASSERT_FALSE(rig.engine.test(2, rig.now, 2000));
    rig.run(CH_rpm, 5000, 1900);
    TEST_ASSERT_TRUE(rig.active(1));
    TEST_ASSERT_EQUAL(0, rig.engine.state(1).count);  // a test is not a firing
    // It ends with the test, without the hold time on top.
    rig.run(CH_rpm, 5000, 300);
    TEST_ASSERT_FALSE(rig.active(1));
}

void test_first_active_rule_wins_per_gauge() {
    Rig rig(R"([{"channel":"rpm","when":"above","value":7000,"hold":0,"message":"SHIFT","gauge":"A0:76:4E:11:22:33"},
                {"channel":"coolant_temp","when":"above","value":105,"hold":0,"message":"HOT","face":2},
                {"channel":"oil_pressure","when":"below","value":100,"hold":0,"face":3}])");
    TEST_ASSERT_EQUAL(-1, rig.engine.messageFor(kMacA));
    TEST_ASSERT_EQUAL(-1, rig.engine.faceFor(kMacA));

    rig.run(CH_coolant_temp, 110, 200);
    rig.run(CH_oil_pressure, 50, 200);
    rig.run(CH_coolant_temp, 110, 100);
    // Both gauges show the coolant message and its face; oil pressure is lower in the list.
    TEST_ASSERT_EQUAL(1, rig.engine.messageFor(kMacA));
    TEST_ASSERT_EQUAL(1, rig.engine.messageFor(kMacB));
    TEST_ASSERT_EQUAL(1, rig.engine.faceFor(kMacB));

    // The shift alert is first in the list but only for gauge A.
    rig.run(CH_rpm, 7500, 100);
    TEST_ASSERT_EQUAL(0, rig.engine.messageFor(kMacA));
    TEST_ASSERT_EQUAL(1, rig.engine.messageFor(kMacB));
    TEST_ASSERT_EQUAL(1, rig.engine.faceFor(kMacA));  // it has no face of its own

    // Coolant stops arriving: after a second it is stale and oil pressure's face takes over.
    for (int i = 0; i < 15; i++) rig.run(CH_oil_pressure, 50, 100);
    TEST_ASSERT_EQUAL(2, rig.engine.faceFor(kMacB));
    TEST_ASSERT_EQUAL(-1, rig.engine.messageFor(kMacB));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_rules_parse_and_write_back);
    RUN_TEST(test_bad_rules_are_refused_and_named);
    RUN_TEST(test_above_fires_after_its_delay_and_holds);
    RUN_TEST(test_thresholds_are_in_the_rules_unit);
    RUN_TEST(test_range_conditions);
    RUN_TEST(test_flags_and_changes);
    RUN_TEST(test_missing_data_ends_an_alert_and_is_not_a_change);
    RUN_TEST(test_disabled_rules_never_fire_and_test_forces_one);
    RUN_TEST(test_first_active_rule_wins_per_gauge);
    return UNITY_END();
}
