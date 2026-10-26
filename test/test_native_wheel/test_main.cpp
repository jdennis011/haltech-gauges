// The steering wheel module's hub-side logic (docs/steering-wheel.md): the
// buttons to outputs with the fail-safe, and the FG Falcon cruise frame table.

#include <string.h>
#include <unity.h>

#include <string>
#include <utility>
#include <vector>

#include "fg_cruise.h"
#include "wheel_controller.h"

using namespace hg;
using namespace hg::proto;
using namespace hg::wheel;

void setUp() {}
void tearDown() {}

namespace {

struct Rec {
    uint32_t now = 1000;
    std::vector<std::pair<uint32_t, bool>> horn;
    std::vector<uint16_t> cruise;
    std::vector<uint16_t> media;
};

void onHorn(bool on, void* ctx) {
    Rec* r = static_cast<Rec*>(ctx);
    r->horn.push_back({r->now, on});
}
void onCruise(uint16_t mask, void* ctx) { static_cast<Rec*>(ctx)->cruise.push_back(mask); }
void onMedia(uint16_t mask, void* ctx) { static_cast<Rec*>(ctx)->media.push_back(mask); }

struct Rig {
    Rec rec;
    Controller c;
    uint8_t seq = 0;
    const uint32_t node = 0x123456;

    explicit Rig(bool enabled = true) {
        Settings s;
        s.hornEnabled = s.cruiseEnabled = s.mediaEnabled = enabled;
        c.init({onHorn, onCruise, onMedia, &rec}, s);
    }
    void input(uint16_t mask, uint8_t flags = 0, uint32_t from = 0) {
        Input in;
        in.seq = seq++;
        in.mask = mask;
        in.flags = flags;
        in.lines = 3;
        c.onInput(from ? from : node, in, rec.now);
    }
    void run(uint32_t ms) {
        for (uint32_t i = 0; i < ms; i++) {
            rec.now++;
            c.poll(rec.now);
        }
    }
    bool hornOn() const { return !rec.horn.empty() && rec.horn.back().second; }
};

}  // namespace

// ------------------------------------------------------------ controller

void test_horn_follows_the_button() {
    Rig r;
    r.input(kInputHorn);
    TEST_ASSERT_EQUAL(1, r.rec.horn.size());
    TEST_ASSERT_TRUE(r.hornOn());
    TEST_ASSERT_TRUE(r.c.state().hornOn);
    TEST_ASSERT_EQUAL(1000, r.rec.horn[0].first);
    r.input(0);
    TEST_ASSERT_EQUAL(2, r.rec.horn.size());
    TEST_ASSERT_FALSE(r.hornOn());
    // Only changes are reported.
    r.input(0);
    TEST_ASSERT_EQUAL(2, r.rec.horn.size());
}

void test_cruise_and_media_masks_reach_their_outputs() {
    Rig r;
    r.input(kInputCruiseSet | kInputVolUp);
    TEST_ASSERT_EQUAL(1, r.rec.cruise.size());
    TEST_ASSERT_EQUAL_HEX16(kInputCruiseSet, r.rec.cruise.back());
    TEST_ASSERT_EQUAL_HEX16(kInputVolUp, r.rec.media.back());
    TEST_ASSERT_TRUE(r.rec.horn.empty());
    r.input(kInputCruiseSet | kInputCruiseAccel);
    TEST_ASSERT_EQUAL_HEX16(kInputCruiseSet | kInputCruiseAccel, r.rec.cruise.back());
    TEST_ASSERT_EQUAL_HEX16(0, r.rec.media.back());
    r.input(0);
    TEST_ASSERT_EQUAL_HEX16(0, r.rec.cruise.back());
}

void test_silence_while_held_releases_at_the_fail_safe() {
    Rig r;
    r.input(kInputHorn | kInputCruiseSet);
    r.run(99);
    TEST_ASSERT_TRUE(r.hornOn());
    TEST_ASSERT_EQUAL(0, r.c.state().failSafeReleases);
    r.run(1);
    TEST_ASSERT_FALSE(r.hornOn());
    TEST_ASSERT_EQUAL_HEX16(0, r.rec.cruise.back());
    TEST_ASSERT_EQUAL(1, r.c.state().failSafeReleases);
    TEST_ASSERT_EQUAL(0, r.c.state().mask);
    r.run(2000);
    TEST_ASSERT_EQUAL(1, r.c.state().failSafeReleases);
    TEST_ASSERT_TRUE(r.c.state().linkLost);
}

void test_idle_silence_does_nothing() {
    Rig r;
    r.input(0);
    r.run(5000);
    TEST_ASSERT_TRUE(r.rec.horn.empty());
    TEST_ASSERT_EQUAL(0, r.c.state().failSafeReleases);
    TEST_ASSERT_TRUE(r.c.state().linkLost);
    r.input(0, kInputFlagHeartbeat);
    TEST_ASSERT_FALSE(r.c.state().linkLost);
}

void test_repeated_reports_keep_a_hold_alive() {
    Rig r;
    r.input(kInputHorn);
    for (int i = 0; i < 50; i++) {
        r.run(20);
        r.input(kInputHorn);
    }
    TEST_ASSERT_TRUE(r.hornOn());
    TEST_ASSERT_EQUAL(1, r.rec.horn.size());
    TEST_ASSERT_EQUAL(0, r.c.state().failSafeReleases);
}

void test_disabled_outputs_never_fire() {
    Rig r(false);
    r.input(kInputHorn | kInputCruiseSet | kInputVolUp);
    r.run(10);
    TEST_ASSERT_TRUE(r.rec.horn.empty());
    TEST_ASSERT_TRUE(r.rec.cruise.empty());
    TEST_ASSERT_TRUE(r.rec.media.empty());
    // Enabling while held applies at once; disabling releases at once.
    Settings s = r.c.settings();
    s.hornEnabled = true;
    r.c.setSettings(s, r.rec.now);
    TEST_ASSERT_TRUE(r.hornOn());
    s.hornEnabled = false;
    r.c.setSettings(s, r.rec.now);
    TEST_ASSERT_FALSE(r.hornOn());
    // A silence with every output disabled is not a fail-safe release.
    s.hornEnabled = true;
    r.c.setSettings(s, r.rec.now);
    r.input(kInputCruiseSet);
    r.run(500);
    TEST_ASSERT_EQUAL(0, r.c.state().failSafeReleases);
}

void test_a_fault_or_no_calibration_blocks_the_horn() {
    Rig r;
    r.input(kInputHorn, kInputFlagFault);
    TEST_ASSERT_TRUE(r.rec.horn.empty());
    r.input(kInputHorn, kInputFlagUncalibrated);
    TEST_ASSERT_TRUE(r.rec.horn.empty());
    r.input(kInputHorn | kInputVolUp, kInputFlagFault);
    TEST_ASSERT_EQUAL_HEX16(kInputVolUp, r.rec.media.back());  // the other outputs still work
    r.input(kInputHorn, 0);
    TEST_ASSERT_TRUE(r.hornOn());
}

void test_test_pulses_time_out_and_are_refused_while_held() {
    Rig r;
    TEST_ASSERT_TRUE(r.c.test(Output::Horn, 0, 200, r.rec.now));
    TEST_ASSERT_TRUE(r.hornOn());
    TEST_ASSERT_FALSE(r.c.test(Output::Horn, 0, 200, r.rec.now));  // one at a time
    r.run(199);
    TEST_ASSERT_TRUE(r.hornOn());
    r.run(1);
    TEST_ASSERT_FALSE(r.hornOn());
    TEST_ASSERT_EQUAL(0, r.c.state().failSafeReleases);  // a pulse is not the wheel's hold

    TEST_ASSERT_TRUE(r.c.test(Output::Cruise, kInputCruiseSet, 100, r.rec.now));
    TEST_ASSERT_EQUAL_HEX16(kInputCruiseSet, r.rec.cruise.back());
    r.run(100);
    TEST_ASSERT_EQUAL_HEX16(0, r.rec.cruise.back());
    TEST_ASSERT_FALSE(r.c.test(Output::Cruise, kInputVolUp, 100, r.rec.now));  // not a cruise button

    r.input(kInputHorn);
    TEST_ASSERT_FALSE(r.c.test(Output::Horn, 0, 200, r.rec.now));  // the wheel holds it
    r.input(0);
    TEST_ASSERT_FALSE(r.c.test(Output::Media, kInputVolUp, 0, r.rec.now));  // no length
}

void test_sequence_gaps_are_counted_but_a_restart_is_not() {
    Rig r;
    r.input(0);
    r.input(0);
    r.seq = 5;
    r.input(0);
    TEST_ASSERT_EQUAL(3, r.c.state().seqGaps);
    r.seq = 0;  // the wheel restarted
    r.input(0);
    TEST_ASSERT_EQUAL(3, r.c.state().seqGaps);
}

void test_a_second_wheel_is_ignored() {
    Rig r;
    r.input(0);
    r.input(kInputHorn, 0, 0x654321);
    TEST_ASSERT_TRUE(r.rec.horn.empty());
    TEST_ASSERT_EQUAL_HEX32(0x123456, r.c.state().node);
}

void test_raw_readings_are_kept_for_the_page() {
    Rig r;
    InputRaw raw;
    raw.first = 0;
    raw.mv[0] = 1200;
    raw.mv[1] = 2500;
    raw.mv[2] = 310;
    r.c.onRaw(r.node, raw, r.rec.now);
    TEST_ASSERT_EQUAL(1200, r.c.state().rawMv[0]);
    TEST_ASSERT_EQUAL(310, r.c.state().rawMv[2]);
    raw.first = 3;
    raw.mv[0] = 77;
    r.c.onRaw(r.node, raw, r.rec.now);
    TEST_ASSERT_EQUAL(77, r.c.state().rawMv[3]);
    TEST_ASSERT_EQUAL(1200, r.c.state().rawMv[0]);
}

void test_reset_turns_everything_off_and_forgets_the_wheel() {
    Rig r;
    r.input(kInputHorn | kInputVolUp);
    r.c.reset(r.rec.now);
    TEST_ASSERT_FALSE(r.hornOn());
    TEST_ASSERT_EQUAL_HEX16(0, r.rec.media.back());
    TEST_ASSERT_FALSE(r.c.state().seen);
    r.input(kInputHorn, 0, 0x654321);  // a new wheel is taken up
    TEST_ASSERT_TRUE(r.hornOn());
}

// ------------------------------------------------------------ FG frame table

FgCruiseFrame sampleTable() {
    FgCruiseFrame t;
    t.id = 0x437;
    t.len = 8;
    t.idle[2] = 0x10;
    t.button[1] = {2, 0x04};  // set
    t.button[3] = {2, 0x08};  // accel
    t.button[0] = {5, 0x80};  // on/off
    t.counterByte = 7;
    t.counterMask = 0xF0;
    return t;
}

void test_fg_encode_places_the_bits_in_the_idle_frame() {
    const FgCruiseFrame t = sampleTable();
    CanFrame f;
    TEST_ASSERT_TRUE(fgEncode(t, 0, 0, f));
    TEST_ASSERT_EQUAL_HEX32(0x437, f.id);
    TEST_ASSERT_FALSE(f.extended);
    TEST_ASSERT_EQUAL(8, f.len);
    TEST_ASSERT_EQUAL_HEX8(0x10, f.data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, f.data[7]);
    TEST_ASSERT_TRUE(fgEncode(t, kInputCruiseSet | kInputVolUp, 3, f));
    TEST_ASSERT_EQUAL_HEX8(0x14, f.data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x30, f.data[7]);
    TEST_ASSERT_TRUE(fgEncode(t, kInputCruiseAccel | kInputCruiseOnOff, 17, f));
    TEST_ASSERT_EQUAL_HEX8(0x18, f.data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x80, f.data[5]);
    TEST_ASSERT_EQUAL_HEX8(0x10, f.data[7]);  // 17 wraps within the four counter bits
    // A button the table does not carry changes nothing.
    TEST_ASSERT_TRUE(fgEncode(t, kInputCruiseCancel, 0, f));
    TEST_ASSERT_EQUAL_HEX8(0x10, f.data[2]);
}

void test_fg_table_without_an_id_or_out_of_range_is_refused() {
    FgCruiseFrame t;
    CanFrame f;
    TEST_ASSERT_FALSE(fgValid(t));
    TEST_ASSERT_FALSE(fgEncode(t, kInputCruiseSet, 0, f));
    t = sampleTable();
    t.len = 4;  // the on/off bit sits in byte 5
    TEST_ASSERT_FALSE(fgValid(t));
    t = sampleTable();
    t.id = 0x800;  // beyond an 11-bit id
    TEST_ASSERT_FALSE(fgValid(t));
    t.extended = true;
    TEST_ASSERT_TRUE(fgValid(t));
}

void test_fg_json_round_trip() {
    const FgCruiseFrame t = sampleTable();
    JsonDocument doc;
    fgToJson(t, doc.to<JsonObject>());
    std::string text;
    serializeJson(doc, text);
    TEST_ASSERT_NOT_NULL(strstr(text.c_str(), "\"id\":\"0x437\""));
    TEST_ASSERT_NOT_NULL(strstr(text.c_str(), "\"idle\":\"00 00 10 00 00 00 00 00\""));

    JsonDocument back;
    TEST_ASSERT_TRUE(deserializeJson(back, text) == DeserializationError::Ok);
    FgCruiseFrame u;
    std::string error;
    TEST_ASSERT_TRUE_MESSAGE(fgFromJson(back.as<JsonVariantConst>(), u, error), error.c_str());
    TEST_ASSERT_EQUAL_HEX32(0x437, u.id);
    TEST_ASSERT_EQUAL(8, u.len);
    TEST_ASSERT_EQUAL(20, u.rateHz);
    TEST_ASSERT_EQUAL_HEX8(0x10, u.idle[2]);
    TEST_ASSERT_EQUAL(2, u.button[1].byte);
    TEST_ASSERT_EQUAL_HEX8(0x04, u.button[1].mask);
    TEST_ASSERT_EQUAL_HEX8(0x80, u.button[0].mask);
    TEST_ASSERT_EQUAL(7, u.counterByte);
    TEST_ASSERT_EQUAL_HEX8(0xF0, u.counterMask);

    // Plain numbers and hex strings both read; a bad id is refused with a reason.
    JsonDocument loose;
    deserializeJson(loose, "{\"id\":1079,\"idle\":\"0010\",\"buttons\":{\"set\":{\"byte\":1,\"mask\":4}},\"counter\":{}}");
    TEST_ASSERT_TRUE(fgFromJson(loose.as<JsonVariantConst>(), u, error));
    TEST_ASSERT_EQUAL_HEX32(0x437, u.id);
    TEST_ASSERT_EQUAL_HEX8(0x10, u.idle[1]);
    TEST_ASSERT_EQUAL(-1, u.counterByte);
    JsonDocument bad;
    deserializeJson(bad, "{\"id\":\"0x4G7\"}");
    TEST_ASSERT_FALSE(fgFromJson(bad.as<JsonVariantConst>(), u, error));
    TEST_ASSERT_TRUE(error.find("id") != std::string::npos);
    JsonDocument empty;
    deserializeJson(empty, "{}");
    TEST_ASSERT_TRUE(fgFromJson(empty.as<JsonVariantConst>(), u, error));
    TEST_ASSERT_EQUAL(0, u.id);  // no id yet: nothing will be sent
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_horn_follows_the_button);
    RUN_TEST(test_cruise_and_media_masks_reach_their_outputs);
    RUN_TEST(test_silence_while_held_releases_at_the_fail_safe);
    RUN_TEST(test_idle_silence_does_nothing);
    RUN_TEST(test_repeated_reports_keep_a_hold_alive);
    RUN_TEST(test_disabled_outputs_never_fire);
    RUN_TEST(test_a_fault_or_no_calibration_blocks_the_horn);
    RUN_TEST(test_test_pulses_time_out_and_are_refused_while_held);
    RUN_TEST(test_sequence_gaps_are_counted_but_a_restart_is_not);
    RUN_TEST(test_a_second_wheel_is_ignored);
    RUN_TEST(test_raw_readings_are_kept_for_the_page);
    RUN_TEST(test_reset_turns_everything_off_and_forgets_the_wheel);
    RUN_TEST(test_fg_encode_places_the_bits_in_the_idle_frame);
    RUN_TEST(test_fg_table_without_an_id_or_out_of_range_is_refused);
    RUN_TEST(test_fg_json_round_trip);
    return UNITY_END();
}
