#include "drag_timer.h"

#include <math.h>
#include <string.h>

namespace hg {
namespace motion {

namespace {

constexpr float kFoot = 0.3048f;
constexpr float kMile = 1609.344f;

struct RunDef {
    const char* key;
    const char* name;
    float startKmh;  // rolling runs: armed below, started across; 0 for a standing start
    Split split[kMaxSplits];
    uint8_t count;
};

const RunDef kRuns[kRunCount] = {
    {"0-100", "0-100 km/h", 0, {{"100 km/h", 0, 100}}, 1},
    {"strip", "1/4 mile", 0, {{"60 ft", 60 * kFoot, 0}, {"330 ft", 330 * kFoot, 0}, {"1/8 mi", kMile / 8, 0}, {"1/4 mi", kMile / 4, 0}}, 4},
    {"80-120", "80-120 km/h", 80, {{"120 km/h", 0, 120}}, 1},
    {"100-200", "100-200 km/h", 100, {{"200 km/h", 0, 200}}, 1},
};

const RunDef& def(Run run) {
    const size_t i = size_t(run);
    return kRuns[i < kRunCount ? i : 0];
}

float ms(float kmh) { return kmh / 3.6f; }
uint32_t roundMs(float seconds) { return uint32_t(lroundf(seconds * 1000.0f)); }

}  // namespace

const Split* splits(Run run, size_t& count) {
    const RunDef& d = def(run);
    count = d.count;
    return d.split;
}
const char* runName(Run run) { return def(run).name; }
const char* runKey(Run run) { return def(run).key; }
bool runFromKey(const char* key, Run& out) {
    if (!key) return false;
    for (size_t i = 0; i < kRunCount; i++) {
        if (strcmp(kRuns[i].key, key) == 0) {
            out = Run(i);
            return true;
        }
    }
    return false;
}
bool runIsRolling(Run run) { return def(run).startKmh > 0; }

bool Result::complete() const { return !aborted && count == def(run).count; }

void DragTimer::configure(Run run, bool rollout) {
    run_ = run;
    rollout_ = rollout && !runIsRolling(run);
    reset();
}

void DragTimer::reset() {
    state_ = State::Idle;
    result_ = Result();
    result_.run = run_;
    haveSample_ = false;
    still_ = false;
    launchGMs_ = 0;
    clockSet_ = false;
    movingSamples_ = 0;
    distance_ = 0;
    peak_ = 0;
    speed_ = 0;
}

void DragTimer::arm(uint32_t) {
    state_ = State::Armed;
    launchGMs_ = 0;
    movingSamples_ = 0;
}

void DragTimer::launch(uint32_t startMs, uint32_t nowMs) {
    state_ = State::Running;
    result_ = Result();
    result_.run = run_;
    result_.seq = ++seq_;
    t0_ = startMs;
    launchMs_ = startMs;
    clockSet_ = !rollout_;
    distance_ = 0;
    peak_ = 0;
    still_ = false;
    (void)nowMs;
}

void DragTimer::finish(bool aborted, uint32_t nowMs) {
    result_.aborted = aborted;
    state_ = aborted ? State::Aborted : State::Done;
    endMs_ = nowMs;
    still_ = false;
    launchGMs_ = 0;
}

void DragTimer::addSplit(uint32_t atMs, float speedMs) {
    if (result_.count >= kMaxSplits) return;
    result_.ms[result_.count] = atMs > t0_ ? atMs - t0_ : 0;
    const float kmh10 = speedMs * 3.6f * 10.0f;
    result_.speed10[result_.count] = uint16_t(kmh10 < 0 ? 0 : (kmh10 > 65535 ? 65535 : lroundf(kmh10)));
    result_.count++;
}

// The samples at tPrev and t, in m/s, with dPrev metres covered by tPrev: finds
// the splits reached between them, the speed taken as changing evenly across
// the interval, so a distance is reached where the quadratic says.
void DragTimer::crossSplits(float vPrev, float v, float dPrev, uint32_t tPrev, uint32_t t) {
    const float dt = float(t - tPrev) / 1000.0f;
    if (dt <= 0) return;
    const float a = (v - vPrev) / dt;
    const float dEnd = dPrev + (vPrev + v) / 2 * dt;
    auto distTime = [&](float D, float& tau, float& vAt) -> bool {
        if (D <= dPrev || D > dEnd) return false;
        const float need = D - dPrev;
        if (fabsf(a) < 1e-3f) {
            tau = need / (vPrev > 1e-3f ? vPrev : 1e-3f);
        } else {
            const float disc = vPrev * vPrev + 2 * a * need;
            tau = (-vPrev + sqrtf(disc > 0 ? disc : 0)) / a;
        }
        if (tau < 0) tau = 0;
        if (tau > dt) tau = dt;
        vAt = vPrev + a * tau;
        return true;
    };
    if (rollout_ && !clockSet_) {
        float tau, vAt;
        if (!distTime(kRolloutM, tau, vAt)) return;  // nothing counts before the beam
        t0_ = tPrev + roundMs(tau);
        clockSet_ = true;
    }
    const RunDef& d = def(run_);
    while (result_.count < d.count) {
        const Split& sp = d.split[result_.count];
        float tau, vAt;
        if (sp.metres > 0) {
            if (!distTime(sp.metres, tau, vAt)) break;
        } else {
            const float target = ms(sp.kmh);
            if (!(vPrev < target && v >= target)) break;
            tau = (target - vPrev) / (v - vPrev) * dt;
            vAt = target;
        }
        addSplit(tPrev + roundMs(tau), vAt);
        if (result_.count == d.count) finish(false, tPrev + roundMs(tau));
    }
}

void DragTimer::onSpeed(float kmh, uint32_t nowMs) {
    const bool first = !haveSample_;
    const float prevV = speed_;
    const uint32_t prevT = sampleMs_;
    speed_ = kmh;
    sampleMs_ = nowMs;
    haveSample_ = true;
    if (first) return;
    const RunDef& d = def(run_);
    const bool rolling = d.startKmh > 0;

    switch (state_) {
        case State::Idle:
        case State::Done:
        case State::Aborted: {
            const float below = rolling ? d.startKmh - 2 : kArmKmh;
            if (kmh < below) {
                if (!still_) {
                    still_ = true;
                    stillSinceMs_ = nowMs;
                }
                if (uint32_t(nowMs - stillSinceMs_) >= kArmMs) arm(nowMs);
            } else {
                still_ = false;
            }
            break;
        }

        case State::Armed:
            if (rolling) {
                if (prevV < d.startKmh && kmh >= d.startKmh) {
                    const float tau = (d.startKmh - prevV) / (kmh - prevV) * float(nowMs - prevT);
                    launch(prevT + uint32_t(lroundf(tau)), nowMs);
                    crossSplits(ms(prevV), ms(kmh), 0, prevT, nowMs);
                    peak_ = kmh;
                } else if (kmh >= d.startKmh) {
                    still_ = false;  // already past it: wait for the next dip below
                    state_ = State::Idle;
                }
            } else if (kmh >= kArmKmh) {
                // Moving. The accelerometer may have seen the launch already;
                // otherwise the start is placed by the next sample.
                firstMoving_ = kmh;
                firstMovingMs_ = nowMs;
                movingSamples_ = 1;
                const uint32_t guess = prevT + (nowMs - prevT) / 2;
                const bool byImu = launchGMs_ != 0 && int32_t(nowMs - launchGMs_) >= 0 && uint32_t(nowMs - launchGMs_) <= 1500;
                launch(byImu ? launchGMs_ : guess, nowMs);
                lastStillMs_ = prevT;
                refined_ = byImu;
                rampPending_ = true;
                peak_ = kmh;
            }
            break;

        case State::Running: {
            uint32_t tPrev = prevT;
            float vPrevMs = ms(prevV);
            if (rampPending_) {
                rampPending_ = false;
                if (!refined_) {
                    // Two moving samples: the speed's slope says when it left zero.
                    refined_ = true;
                    const float slope = (kmh - firstMoving_) / float(nowMs - firstMovingMs_);  // km/h per ms
                    if (slope > 0) {
                        // A sensor that reads 0 below a few km/h puts the start before
                        // its last zero, so only a wild slope is reined in.
                        uint32_t back = uint32_t(lroundf(firstMoving_ / slope));
                        if (back > 1000) back = 1000;
                        uint32_t t0 = firstMovingMs_ - back;
                        t0_ = t0;
                        launchMs_ = t0;
                    }
                }
                // From rest at the start to the first moving sample.
                crossSplits(0, ms(firstMoving_), 0, t0_, firstMovingMs_);
                distance_ = ms(firstMoving_) / 2 * float(firstMovingMs_ - t0_) / 1000.0f;
                tPrev = firstMovingMs_;
                vPrevMs = ms(firstMoving_);
                if (state_ != State::Running) break;
            }
            const float vMs = ms(kmh);
            const float dPrev = distance_;
            distance_ = dPrev + (vPrevMs + vMs) / 2 * float(nowMs - tPrev) / 1000.0f;
            crossSplits(vPrevMs, vMs, dPrev, tPrev, nowMs);
            if (state_ != State::Running) break;
            if (kmh > peak_) peak_ = kmh;
            if (kmh < peak_ - kAbortDropKmh) finish(true, nowMs);
            break;
        }
    }
}

void DragTimer::onAccel(float lonG, uint32_t nowMs) {
    if (state_ == State::Armed && !runIsRolling(run_) && launchGMs_ == 0 && lonG >= kLaunchG) launchGMs_ = nowMs;
}

void DragTimer::poll(uint32_t nowMs) {
    switch (state_) {
        case State::Running:
            if (uint32_t(nowMs - sampleMs_) > kStaleMs || uint32_t(nowMs - launchMs_) > kTimeoutMs) finish(true, nowMs);
            break;
        case State::Armed:
            // A bump that moved the accelerometer but not the car is forgotten.
            if (launchGMs_ && uint32_t(nowMs - launchGMs_) > 1500) launchGMs_ = 0;
            break;
        default:
            if (still_ && haveSample_ && uint32_t(nowMs - stillSinceMs_) >= kArmMs) arm(nowMs);
            break;
    }
}

uint32_t DragTimer::elapsedMs(uint32_t nowMs) const {
    switch (state_) {
        case State::Running: return clockSet_ ? nowMs - t0_ : 0;
        case State::Done: return result_.totalMs();
        case State::Aborted: return clockSet_ && int32_t(endMs_ - t0_) > 0 ? endMs_ - t0_ : 0;
        default: return 0;
    }
}

}  // namespace motion
}  // namespace hg
