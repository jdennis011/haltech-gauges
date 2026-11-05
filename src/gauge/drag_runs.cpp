#include "drag_runs.h"

#include <math.h>

#include "can_task.h"
#include "imu.h"

using hg::motion::DragTimer;
using hg::motion::Result;
using hg::motion::Run;

namespace drag_runs {

namespace {

struct Slot {
    bool used = false;
    bool rollout = false;
    DragTimer timer;
    DragTimer::State was = DragTimer::State::Idle;
    Memory memory;
};
Slot slots[kRunCount];
uint32_t lastStampMs = 0;
bool haveStamp = false;
bool demoWas = false;
bool wasActive = false;
uint32_t demoSampleMs = 0;
uint32_t demoStartMs = 0;

// The demo's speed trace, km/h at `t` ms into a 24 second cycle: three seconds
// at rest, a pull to about 190 km/h, a stop, and rest again to re-arm.
float demoSpeed(uint32_t t) {
    const float s = float(t % 24000) / 1000.0f;
    if (s < 3) return 0;
    if (s < 16) return 190.0f * (1.0f - expf(-(s - 3) / 5.0f));
    const float v16 = 190.0f * (1.0f - expf(-13.0f / 5.0f));
    const float v = v16 - 60.0f * (s - 16);
    return v > 0 ? v : 0;
}

void feed(float kmh, uint32_t stampMs) {
    for (Slot& s : slots) {
        if (s.used) s.timer.onSpeed(kmh, stampMs);
    }
}

void finished(Slot& s, bool demo) {
    const Result& r = s.timer.result();
    if (demo) return;
    s.memory.haveLast = true;
    s.memory.last = r;
    if (r.complete() && (!s.memory.haveBest || r.totalMs() < s.memory.bestMs)) {
        s.memory.haveBest = true;
        s.memory.bestMs = r.totalMs();
    }
    can_task::sendRun(r);
}

}  // namespace

void setRuns(uint8_t mask, const bool rollout[kRunCount]) {
    for (size_t i = 0; i < kRunCount; i++) {
        Slot& s = slots[i];
        const bool want = (mask >> i) & 1u;
        if (want && (!s.used || s.rollout != rollout[i])) {
            // New, or a changed rollout: a fresh timer. Nothing else restarts one.
            s.timer.configure(Run(i), rollout[i]);
            s.rollout = rollout[i];
            s.was = DragTimer::State::Idle;
            s.used = true;
        } else if (!want && s.used) {
            s.used = false;
        }
    }
}

bool active() {
    for (const Slot& s : slots) {
        if (s.used) return true;
    }
    return false;
}

void poll(uint32_t nowMs, bool demo) {
    if (!active()) {
        wasActive = false;
        return;
    }
    // A fresh start when the timers come into use, or the demo comes or goes.
    if (!wasActive || demo != demoWas) {
        wasActive = true;
        demoWas = demo;
        haveStamp = false;
        for (Slot& s : slots) {
            if (s.used) s.timer.reset();
        }
        demoStartMs = nowMs;
        demoSampleMs = nowMs;
    }
    if (demo) {
        // Twenty samples a second, like the bus.
        while (uint32_t(nowMs - demoSampleMs) >= 50) {
            demoSampleMs += 50;
            feed(demoSpeed(demoSampleMs - demoStartMs), demoSampleMs);
        }
    } else {
        float kmh;
        uint32_t stamp;
        if (can_task::channels().latest(hg::CH_vehicle_speed, kmh, stamp) && (!haveStamp || stamp != lastStampMs)) {
            haveStamp = true;
            lastStampMs = stamp;
            feed(kmh, stamp);
        }
        if (imu::present()) {
            const float lon = imu::lonRaw();
            for (Slot& s : slots) {
                if (s.used) s.timer.onAccel(lon, nowMs);
            }
        }
    }
    for (Slot& s : slots) {
        if (!s.used) continue;
        s.timer.poll(nowMs);
        const DragTimer::State now = s.timer.state();
        if (now != s.was) {
            if (now == DragTimer::State::Done || now == DragTimer::State::Aborted) finished(s, demo);
            s.was = now;
        }
    }
}

const DragTimer* timer(Run run) {
    const size_t i = size_t(run);
    return i < kRunCount && slots[i].used ? &slots[i].timer : nullptr;
}

Memory memory(Run run) {
    const size_t i = size_t(run);
    return i < kRunCount ? slots[i].memory : Memory();
}

void reset() {
    for (Slot& s : slots) s.memory = Memory();
}

}  // namespace drag_runs
