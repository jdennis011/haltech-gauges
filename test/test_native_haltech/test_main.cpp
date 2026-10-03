#include <math.h>
#include <string.h>
#include <unity.h>

#include "haltech.h"
#include "units.h"

using namespace hg;

void setUp() {}
void tearDown() {}

namespace {

struct Decoded {
    float value[CH_COUNT];
    bool valid[CH_COUNT];
    bool seen[CH_COUNT];
    int count;
};

void sink(ChannelId id, float value, bool valid, void* ctx) {
    Decoded* d = static_cast<Decoded*>(ctx);
    d->value[id] = value;
    d->valid[id] = valid;
    d->seen[id] = true;
    d->count++;
}

Decoded decode(uint32_t id, const uint8_t* data, uint8_t len = 8, bool extended = false) {
    CanFrame f;
    f.id = id;
    f.extended = extended;
    f.len = len;
    memcpy(f.data, data, len);
    Decoded d = {};
    decodeFrame(f, sink, &d);
    return d;
}

}  // namespace

void test_table_is_grouped_and_sorted() {
    size_t total = 0;
    for (size_t i = 0; i < frameCount(); i++) {
        const FrameDef& f = frameDef(i);
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, f.channelCount, "frame with no channels");
        TEST_ASSERT_GREATER_THAN(0, f.rateHz);
        if (i > 0) TEST_ASSERT_GREATER_THAN(frameDef(i - 1).canId, f.canId);
        for (uint16_t c = 0; c < f.channelCount; c++) {
            TEST_ASSERT_EQUAL_HEX16(f.canId, channelDef(ChannelId(f.firstChannel + c)).canId);
        }
        total += f.channelCount;
    }
    // Every channel belongs to a declared frame.
    TEST_ASSERT_EQUAL(CH_COUNT, total);
}

void test_channel_names_are_unique() {
    for (int i = 0; i < CH_COUNT; i++) {
        TEST_ASSERT_EQUAL_MESSAGE(i, findChannel(channelDef(ChannelId(i)).name),
                                  channelDef(ChannelId(i)).name);
    }
    TEST_ASSERT_EQUAL(CH_oil_pressure, findChannel("oil_pressure"));
    TEST_ASSERT_EQUAL(-1, findChannel("no_such_channel"));
    TEST_ASSERT_EQUAL(-1, findChannel(nullptr));
}

void test_fields_fit_in_a_frame() {
    for (int i = 0; i < CH_COUNT; i++) {
        const ChannelDef& d = channelDef(ChannelId(i));
        TEST_ASSERT_TRUE_MESSAGE(d.width >= 1 && d.width <= 32, d.name);
        TEST_ASSERT_TRUE_MESSAGE(d.startBit + d.width <= 64, d.name);
        TEST_ASSERT_TRUE_MESSAGE(d.scale != 0.0f, d.name);
    }
}

void test_decode_0x360() {
    // RPM 3500, MAP 101.3 kPa abs, TPS 45.6 %, coolant pressure 100.0 kPa gauge (raw 2013)
    const uint8_t data[8] = {0x0D, 0xAC, 0x03, 0xF5, 0x01, 0xC8, 0x07, 0xDD};
    Decoded d = decode(0x360, data);
    TEST_ASSERT_EQUAL(4, d.count);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3500.0f, d.value[CH_rpm]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 101.3f, d.value[CH_manifold_pressure]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 45.6f, d.value[CH_throttle_position]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, d.value[CH_coolant_pressure]);
    TEST_ASSERT_TRUE(d.valid[CH_rpm]);
}

void test_decode_signed() {
    // Ignition angle -12.3 deg = raw -123 = 0xFF85
    const uint8_t data[8] = {0, 0, 0, 0, 0xFF, 0x85, 0, 0};
    Decoded d = decode(0x362, data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -12.3f, d.value[CH_ignition_angle]);
}

void test_decode_temperature_is_celsius() {
    // Haltech: raw 2731 = 0 C, 3731 = 100 C. Coolant 90.0 C = 3631 = 0x0E2F.
    const uint8_t data[8] = {0x0E, 0x2F, 0x0A, 0xAB, 0, 0, 0x0E, 0x93};
    Decoded d = decode(0x3E0, data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, d.value[CH_coolant_temp]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, d.value[CH_air_temp]);     // 2731
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, d.value[CH_oil_temp]);   // 3731
}

void test_decode_status_bits() {
    uint8_t data[8] = {0, 0x01, 0x20, 0x40, 0xFE, 0, 0, 0x81};
    Decoded d = decode(0x3E4, data);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, d.value[CH_oil_pressure_light]);      // 1:0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, d.value[CH_neutral_switch]);          // 1:7
    TEST_ASSERT_EQUAL_FLOAT(1.0f, d.value[CH_aux_rpm_limiter_active]);  // 2:5
    TEST_ASSERT_EQUAL_FLOAT(1.0f, d.value[CH_traction_control_active]); // 3:6
    TEST_ASSERT_EQUAL_FLOAT(0.0f, d.value[CH_traction_control_enabled]);// 3:7
    TEST_ASSERT_EQUAL_FLOAT(-2.0f, d.value[CH_rotary_trim_pot_1]);      // int8
    TEST_ASSERT_EQUAL_FLOAT(1.0f, d.value[CH_check_engine_light]);      // 7:7
    TEST_ASSERT_EQUAL_FLOAT(1.0f, d.value[CH_traction_control_light]);  // 7:0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, d.value[CH_battery_light]);           // 7:6
}

void test_decode_lights_and_engine_state() {
    // Head light is bit 1 of byte 0; engine state is the low nibble of byte 1.
    const uint8_t data[8] = {0x02, 0x03, 0, 0, 0, 0, 0, 0};
    Decoded d = decode(0x6F4, data);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, d.value[CH_park_light]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, d.value[CH_head_light]);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, d.value[CH_engine_state]);
}

void test_decode_multi_bit_fields() {
    // Cruise state = high nibble of byte 6; input state = 12 bits from 6:3 to 7:0.
    const uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0x21, 0x48};
    Decoded d = decode(0x472, data);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, d.value[CH_cruise_state]);
    TEST_ASSERT_EQUAL_FLOAT(328.0f, d.value[CH_cruise_input_state]);
}

void test_decode_gear() {
    const uint8_t data[8] = {0x03, 0xE8, 0, 0, 0, 0, 0xFC, 0xFF};
    Decoded d = decode(0x470, data);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, d.value[CH_wideband_overall]);
    TEST_ASSERT_EQUAL_FLOAT(-4.0f, d.value[CH_gear_selector]);  // drive
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, d.value[CH_gear]);           // reverse
}

void test_decode_32_bit() {
    const uint8_t data[8] = {0x00, 0x01, 0x86, 0xA0, 0, 0, 0, 0};
    Decoded d = decode(0x3EB, data);
    TEST_ASSERT_EQUAL_FLOAT(100000.0f, d.value[CH_race_timer]);
}

void test_decode_odd_scalings() {
    // Brake pressure has no /10; NOS pressure is x*11/50 - 101.3.
    const uint8_t data[8] = {0x04, 0x4D, 0x13, 0x88, 0x00, 0x64, 0, 0};
    Decoded d = decode(0x36B, data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 999.7f, d.value[CH_brake_pressure_front]);  // 1101
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 998.7f, d.value[CH_nos_pressure_1]);        // 5000
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1000.0f, d.value[CH_turbo_speed_1]);        // 100
}

void test_zero_means_error_on_flagged_channels() {
    const uint8_t data[8] = {0, 0, 0x0C, 0x80, 0, 0, 0, 0};
    Decoded d = decode(0x6F0, data);
    TEST_ASSERT_FALSE(d.valid[CH_tyre_pressure_fl]);
    TEST_ASSERT_TRUE(d.valid[CH_tyre_pressure_fr]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 218.7f, d.value[CH_tyre_pressure_fr]);  // 3200
}

void test_short_frame_reports_only_complete_fields() {
    const uint8_t data[8] = {0x0D, 0xAC, 0x03, 0xF5, 0, 0, 0, 0};
    Decoded d = decode(0x360, data, 4);
    TEST_ASSERT_EQUAL(2, d.count);
    TEST_ASSERT_TRUE(d.seen[CH_rpm]);
    TEST_ASSERT_FALSE(d.seen[CH_throttle_position]);
}

void test_unknown_and_extended_ids_are_ignored() {
    const uint8_t data[8] = {};
    TEST_ASSERT_EQUAL(0, decode(0x123, data).count);
    TEST_ASSERT_EQUAL(0, decode(0x360, data, 8, true).count);
    TEST_ASSERT_NULL(findFrame(0x700));
    TEST_ASSERT_NOT_NULL(findFrame(0x701));
}

void test_encode_round_trips_every_channel() {
    for (size_t fi = 0; fi < frameCount(); fi++) {
        const FrameDef& fd = frameDef(fi);
        CanFrame frame;
        beginFrame(fd, frame);
        float want[64];
        for (uint16_t c = 0; c < fd.channelCount; c++) {
            const ChannelId id = ChannelId(fd.firstChannel + c);
            const ChannelDef& d = channelDef(id);
            // A raw value that fits any field width, negative where allowed.
            double raw = d.width == 1 ? 1 : (d.isSigned ? -3 : 5);
            if (!d.isSigned && d.width >= 8) raw = 5 + c;
            want[c] = float(raw * double(d.scale) + double(d.offset));
            TEST_ASSERT_TRUE_MESSAGE(encodeChannel(id, want[c], frame), d.name);
        }
        Decoded got = {};
        TEST_ASSERT_EQUAL(fd.channelCount, decodeFrame(frame, sink, &got));
        for (uint16_t c = 0; c < fd.channelCount; c++) {
            const ChannelId id = ChannelId(fd.firstChannel + c);
            const ChannelDef& d = channelDef(id);
            TEST_ASSERT_FLOAT_WITHIN_MESSAGE(fabsf(d.scale) * 0.01f + 0.001f, want[c], got.value[id],
                                             d.name);
        }
    }
}

void test_encode_clamps_and_rejects_wrong_frame() {
    CanFrame frame;
    beginFrame(*findFrame(0x360), frame);
    TEST_ASSERT_TRUE(encodeChannel(CH_rpm, 1e9f, frame));
    TEST_ASSERT_EQUAL_HEX8(0xFF, frame.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, frame.data[1]);
    TEST_ASSERT_FALSE(encodeChannel(CH_oil_pressure, 100.0f, frame));  // lives in 0x361
}

void test_display_units() {
    const DisplayUnit* psi = findDisplayUnit(Unit::KpaAbs, "psi_gauge");
    TEST_ASSERT_NOT_NULL(psi);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 14.50f, toDisplay(*psi, 201.3f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, toDisplay(*psi, 101.3f));

    const DisplayUnit* f = findDisplayUnit(Unit::Celsius, "F");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 212.0f, toDisplay(*f, 100.0f));

    const DisplayUnit* afr = findDisplayUnit(Unit::Lambda, "afr");
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 14.7f, toDisplay(*afr, 1.0f));

    // No name gives the native unit; a unit from another family is refused.
    const DisplayUnit* native = findDisplayUnit(Unit::Rpm, nullptr);
    TEST_ASSERT_NOT_NULL(native);
    TEST_ASSERT_EQUAL_STRING("rpm", native->symbol);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, native->mul);
    TEST_ASSERT_NULL(findDisplayUnit(Unit::Rpm, "psi"));
    TEST_ASSERT_NULL(findDisplayUnit(Unit::Kpa, "psi_gauge"));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_table_is_grouped_and_sorted);
    RUN_TEST(test_channel_names_are_unique);
    RUN_TEST(test_fields_fit_in_a_frame);
    RUN_TEST(test_decode_0x360);
    RUN_TEST(test_decode_signed);
    RUN_TEST(test_decode_temperature_is_celsius);
    RUN_TEST(test_decode_status_bits);
    RUN_TEST(test_decode_lights_and_engine_state);
    RUN_TEST(test_decode_multi_bit_fields);
    RUN_TEST(test_decode_gear);
    RUN_TEST(test_decode_32_bit);
    RUN_TEST(test_decode_odd_scalings);
    RUN_TEST(test_zero_means_error_on_flagged_channels);
    RUN_TEST(test_short_frame_reports_only_complete_fields);
    RUN_TEST(test_unknown_and_extended_ids_are_ignored);
    RUN_TEST(test_encode_round_trips_every_channel);
    RUN_TEST(test_encode_clamps_and_rejects_wrong_frame);
    RUN_TEST(test_display_units);
    return UNITY_END();
}
