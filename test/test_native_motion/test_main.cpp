// The G meter's orientation maths and the drag timer, on made-up traces.
#include <math.h>
#include <string.h>
#include <unity.h>

#include "drag_timer.h"
#include "motion.h"

using namespace hg::motion;

void setUp() {}
void tearDown() {}

namespace {

// A car that starts at `startMs` and accelerates evenly at `a` m/s2, read
// every 50 ms from t = 0. Speed readings below `floorKmh` come out as 0, as a
// wheel speed sensor's do.
struct Trace {
    float a = 5;
    uint32_t startMs = 2000;
    float floorKmh = 0;
    float speedAt(uint32_t t) const {
        if (t <= startMs) return 0;
        const float v = a * float(t - startMs) / 1000.0f * 3.6f;
        return v < floorKmh ? 0 : v;
    }
};

// Feeds the timer until `untilMs`, polling between samples.
void drive(DragTimer& dt, const Trace& tr, uint32_t untilMs, bool imu = false) {
    for (uint32_t t = 0; t <= untilMs; t += 10) {
        if (imu && t >= tr.startMs && t % 10 == 0) dt.onAccel(t > tr.startMs ? tr.a / 9.80665f : 0, t);
        if (t % 50 == 0) dt.onSpeed(tr.speedAt(t), t);
        dt.poll(t);
    }
}

// The time the trace reaches `metres` from the start, in ms from the start.
uint32_t timeToMetres(const Trace& tr, float metres) { return uint32_t(lroundf(sqrtf(2 * metres / tr.a) * 1000)); }
uint32_t timeToKmh(const Trace& tr, float kmh) { return uint32_t(lroundf(kmh / 3.6f / tr.a * 1000)); }

}  // namespace

// ------------------------------------------------------------ orientation

void test_level_and_forward_captures() {
    Orientation o;
    // Flat on its back: the resting reading points along z. Not 1 g: refused.
    TEST_ASSERT_FALSE(setLevel(o, {0, 0, 2}, {0, -1, 0}));
    TEST_ASSERT_FALSE(o.haveLevel);
    TEST_ASSERT_TRUE(setLevel(o, {0, 0, 1}, {0, -1, 0}));
    TEST_ASSERT_TRUE(o.haveLevel);
    TEST_ASSERT_FALSE(o.haveForward);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, -1, o.forward.y);  // the board's guess, flattened
    // Braking: the push is along +x, so forward is -x.
    TEST_ASSERT_FALSE(setForward(o, {0.05f, 0, 1}));  // not hard enough
    TEST_ASSERT_TRUE(setForward(o, {0.4f, 0, 1}));
    TEST_ASSERT_TRUE(o.haveForward);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, -1, o.forward.x);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0, o.forward.z);
    const Vec3 r = o.right();
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1, r.y);
    // Now a right turn pushes along +y, acceleration along -x.
    CarAccel a = resolve(o, {0, 0.3f, 1});
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.3f, a.lat);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0, a.lon);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0, a.vert);
    a = resolve(o, {-0.2f, 0, 1.1f});
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.2f, a.lon);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.1f, a.vert);
}

void test_tilted_mount_resolves_the_same() {
    // The gauge leans back 30 degrees in its pod: gravity has a y component.
    const float s = sinf(30 * 3.14159265f / 180), c = cosf(30 * 3.14159265f / 180);
    Orientation o;
    TEST_ASSERT_TRUE(setLevel(o, {0, s, c}, {0, 0, -1}));
    // Braking pushes the car backward along the world's horizontal: in the
    // sensor frame that is a mix of y and z, with the gravity still there.
    TEST_ASSERT_TRUE(setForward(o, {0, 0.3f * c + s, -0.3f * s + c}));
    CarAccel a = resolve(o, {0, 0.3f * c + s, -0.3f * s + c});
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, -0.3f, a.lon);  // braking
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0, a.lat);
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0, a.vert);
    a = resolve(o, {0.5f, s, c});
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0.5f, fabsf(a.lat));
    // A new level keeps the captured forward.
    TEST_ASSERT_TRUE(setLevel(o, {0, s, c}, {1, 0, 0}));
    TEST_ASSERT_TRUE(o.haveForward);
    a = resolve(o, {0, 0.3f * c + s, -0.3f * s + c});
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, -0.3f, a.lon);
}

void test_low_pass_settles() {
    LowPass lp(5);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1, lp.step(1, 0.01f));  // primes to the first sample
    float y = 0;
    for (int i = 0; i < 100; i++) y = lp.step(0, 0.01f);
    TEST_ASSERT_TRUE(y < 0.05f);
    TEST_ASSERT_TRUE(y > 0);
}

// ------------------------------------------------------------ drag timer

void test_standing_start_to_100() {
    DragTimer dt(Run::ZeroTo100);
    Trace tr;
    drive(dt, tr, 1500);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Armed), int(dt.state()));
    drive(dt, tr, 9000);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Done), int(dt.state()));
    const Result& r = dt.result();
    TEST_ASSERT_TRUE(r.complete());
    TEST_ASSERT_EQUAL(1, r.count);
    TEST_ASSERT_UINT32_WITHIN(5, timeToKmh(tr, 100), r.ms[0]);
    TEST_ASSERT_UINT16_WITHIN(2, 1000, r.speed10[0]);
    TEST_ASSERT_EQUAL(1, r.seq);
    TEST_ASSERT_EQUAL(r.ms[0], dt.elapsedMs(9000));
}

void test_strip_splits_and_trap_speed() {
    DragTimer dt(Run::Strip);
    Trace tr;
    drive(dt, tr, 20000);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Done), int(dt.state()));
    const Result& r = dt.result();
    TEST_ASSERT_EQUAL(4, r.count);
    TEST_ASSERT_UINT32_WITHIN(6, timeToMetres(tr, 60 * 0.3048f), r.ms[0]);
    TEST_ASSERT_UINT32_WITHIN(6, timeToMetres(tr, 330 * 0.3048f), r.ms[1]);
    TEST_ASSERT_UINT32_WITHIN(6, timeToMetres(tr, 1609.344f / 8), r.ms[2]);
    TEST_ASSERT_UINT32_WITHIN(6, timeToMetres(tr, 1609.344f / 4), r.ms[3]);
    const float trap = tr.a * float(r.ms[3]) / 1000.0f * 3.6f;
    TEST_ASSERT_UINT16_WITHIN(15, uint16_t(trap * 10), r.speed10[3]);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 1609.344f / 4, dt.distanceM() > 1609.344f / 4 ? 1609.344f / 4 : dt.distanceM());
}

void test_rollout_starts_the_clock_a_foot_on() {
    DragTimer plain(Run::Strip, false), rolled(Run::Strip, true);
    Trace tr;
    drive(plain, tr, 20000);
    drive(rolled, tr, 20000);
    TEST_ASSERT_EQUAL(4, rolled.result().count);
    const uint32_t shift = timeToMetres(tr, 0.3048f);
    TEST_ASSERT_UINT32_WITHIN(6, plain.result().ms[0] - shift, rolled.result().ms[0]);
    TEST_ASSERT_UINT32_WITHIN(6, plain.result().ms[3] - shift, rolled.result().ms[3]);
}

void test_rolling_run() {
    DragTimer dt(Run::Rolling80to120);
    Trace tr;
    tr.a = 3;
    drive(dt, tr, 1500);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Armed), int(dt.state()));  // below 78 for a second
    drive(dt, tr, 20000);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Done), int(dt.state()));
    TEST_ASSERT_UINT32_WITHIN(5, timeToKmh(tr, 120) - timeToKmh(tr, 80), dt.result().ms[0]);
    TEST_ASSERT_TRUE(dt.result().complete());
}

void test_lift_aborts_the_run() {
    DragTimer dt(Run::ZeroTo100);
    Trace tr;
    drive(dt, tr, 5000);  // about 54 km/h
    TEST_ASSERT_EQUAL(int(DragTimer::State::Running), int(dt.state()));
    float v = tr.speedAt(5000);
    for (uint32_t t = 5050; t <= 7000; t += 50) {
        v -= 1;
        dt.onSpeed(v, t);
        dt.poll(t);
        if (dt.state() != DragTimer::State::Running) break;
    }
    TEST_ASSERT_EQUAL(int(DragTimer::State::Aborted), int(dt.state()));
    TEST_ASSERT_TRUE(dt.result().aborted);
    TEST_ASSERT_EQUAL(0, dt.result().count);
    TEST_ASSERT_FALSE(dt.result().complete());
    TEST_ASSERT_TRUE(dt.elapsedMs(8000) > 3000 && dt.elapsedMs(8000) < 3400);
}

void test_lost_speed_data_aborts_and_the_car_re_arms() {
    DragTimer dt(Run::Strip);
    Trace tr;
    drive(dt, tr, 5000);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Running), int(dt.state()));
    TEST_ASSERT_EQUAL(1, dt.splitsDone());  // past 60 ft
    for (uint32_t t = 5010; t <= 6500; t += 10) dt.poll(t);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Aborted), int(dt.state()));
    TEST_ASSERT_EQUAL(1, dt.result().count);
    // Stopped again for a second: armed, and a new run counts up from a new start.
    for (uint32_t t = 6550; t <= 8000; t += 50) {
        dt.onSpeed(0, t);
        dt.poll(t);
    }
    TEST_ASSERT_EQUAL(int(DragTimer::State::Armed), int(dt.state()));
    Trace again;
    again.startMs = 9000;
    for (uint32_t t = 8050; t <= 30000; t += 50) {
        dt.onSpeed(again.speedAt(t), t);
        dt.poll(t);
    }
    TEST_ASSERT_EQUAL(int(DragTimer::State::Done), int(dt.state()));
    TEST_ASSERT_EQUAL(2, dt.result().seq);
    TEST_ASSERT_UINT32_WITHIN(6, timeToMetres(again, 60 * 0.3048f), dt.result().ms[0]);
}

void test_accelerometer_places_the_start_before_the_speed_sensor_sees_it() {
    // A sensor that reads 0 below 3 km/h: the first moving sample comes late,
    // and the slope of the two that follow still points back to the start.
    Trace tr;
    tr.floorKmh = 3;
    DragTimer bySpeed(Run::ZeroTo100), byImu(Run::ZeroTo100);
    drive(bySpeed, tr, 9000, false);
    drive(byImu, tr, 9000, true);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Done), int(bySpeed.state()));
    TEST_ASSERT_EQUAL(int(DragTimer::State::Done), int(byImu.state()));
    TEST_ASSERT_UINT32_WITHIN(12, timeToKmh(tr, 100), bySpeed.result().ms[0]);
    TEST_ASSERT_UINT32_WITHIN(12, timeToKmh(tr, 100), byImu.result().ms[0]);
    // A bump while armed, with the car not moving, is forgotten.
    DragTimer bumped(Run::ZeroTo100);
    Trace late;
    late.startMs = 6000;
    drive(bumped, late, 2000);
    bumped.onAccel(0.5f, 2000);
    drive(bumped, late, 12000);
    TEST_ASSERT_EQUAL(int(DragTimer::State::Done), int(bumped.state()));
    TEST_ASSERT_UINT32_WITHIN(12, timeToKmh(late, 100), bumped.result().ms[0]);
}

void test_run_keys_and_names() {
    Run r;
    TEST_ASSERT_TRUE(runFromKey("strip", r));
    TEST_ASSERT_EQUAL(int(Run::Strip), int(r));
    TEST_ASSERT_TRUE(runFromKey("100-200", r));
    TEST_ASSERT_TRUE(runIsRolling(r));
    TEST_ASSERT_FALSE(runFromKey("0-60", r));
    TEST_ASSERT_EQUAL_STRING("0-100", runKey(Run::ZeroTo100));
    TEST_ASSERT_EQUAL_STRING("1/4 mile", runName(Run::Strip));
    size_t n = 0;
    const Split* s = splits(Run::Strip, n);
    TEST_ASSERT_EQUAL(4, n);
    TEST_ASSERT_EQUAL_STRING("60 ft", s[0].name);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 402.336f, s[3].metres);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_level_and_forward_captures);
    RUN_TEST(test_tilted_mount_resolves_the_same);
    RUN_TEST(test_low_pass_settles);
    RUN_TEST(test_standing_start_to_100);
    RUN_TEST(test_strip_splits_and_trap_speed);
    RUN_TEST(test_rollout_starts_the_clock_a_foot_on);
    RUN_TEST(test_rolling_run);
    RUN_TEST(test_lift_aborts_the_run);
    RUN_TEST(test_lost_speed_data_aborts_and_the_car_re_arms);
    RUN_TEST(test_accelerometer_places_the_start_before_the_speed_sensor_sees_it);
    RUN_TEST(test_run_keys_and_names);
    return UNITY_END();
}
