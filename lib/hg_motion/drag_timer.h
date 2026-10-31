#pragma once

#include <stddef.h>
#include <stdint.h>

// The drag timer (docs/motion.md): standing-start and rolling runs timed from
// the ECU's vehicle speed, with the distance splits of a strip. Fed one speed
// sample per frame off the bus, and the longitudinal g from the gauge's own
// accelerometer, which brings the start forward. No hardware here, so the
// host tests drive it with made-up traces.
namespace hg {
namespace motion {

enum class Run : uint8_t {
    ZeroTo100 = 0,       // standing start to 100 km/h
    Strip = 1,           // 60 ft, 330 ft, 1/8 mile, 1/4 mile with the trap speed
    Rolling80to120 = 2,  // from below 80 km/h: the time from 80 to 120
    Rolling100to200 = 3,
};
constexpr size_t kRunCount = 4;
constexpr size_t kMaxSplits = 4;

// One split of a run: a distance from the start, or a speed, never both.
struct Split {
    const char* name;
    float metres;
    float kmh;
};
const Split* splits(Run run, size_t& count);
// "0-100 km/h", "1/4 mile", ... for the screen; "0-100", "strip", "80-120",
// "100-200" in a config.
const char* runName(Run run);
const char* runKey(Run run);
bool runFromKey(const char* key, Run& out);
bool runIsRolling(Run run);

struct Result {
    Run run = Run::ZeroTo100;
    uint8_t seq = 0;                   // counts the runs since power-on
    uint8_t count = 0;                 // splits reached
    uint32_t ms[kMaxSplits] = {};      // each, from the start
    uint16_t speed10[kMaxSplits] = {}; // the speed as it was reached, 0.1 km/h
    bool aborted = false;
    uint32_t totalMs() const { return count ? ms[count - 1] : 0; }
    bool complete() const;
};

class DragTimer {
public:
    enum class State : uint8_t { Idle, Armed, Running, Done, Aborted };

    // A standing start is armed after kArmMs below kArmKmh. It starts on the
    // first of: a speed sample above kArmKmh (the start is then placed by
    // the two readings that follow), or kLaunchG forward from the
    // accelerometer. A rolling run is armed below its first speed and starts
    // as the speed crosses it.
    static constexpr float kArmKmh = 0.5f;
    static constexpr float kLaunchG = 0.2f;
    static constexpr uint32_t kArmMs = 1000;
    // A run is abandoned when the speed falls this far below its peak, after
    // this long, or when no speed sample has come for this long.
    static constexpr float kAbortDropKmh = 5;
    static constexpr uint32_t kTimeoutMs = 30000;
    static constexpr uint32_t kStaleMs = 1000;
    // With rollout, a standing run's clock starts after this much travel, as a
    // strip's beams do.
    static constexpr float kRolloutM = 0.3048f;

    explicit DragTimer(Run run = Run::ZeroTo100, bool rollout = false) { configure(run, rollout); }
    void configure(Run run, bool rollout);
    Run run() const { return run_; }

    // A new vehicle speed sample from the bus, in km/h, stamped when it arrived.
    void onSpeed(float kmh, uint32_t nowMs);
    // The longitudinal acceleration in g (forward positive), at any rate.
    void onAccel(float lonG, uint32_t nowMs);
    // Timeouts; call often.
    void poll(uint32_t nowMs);

    State state() const { return state_; }
    // The last run's result, kept until the next one starts.
    bool haveResult() const { return state_ == State::Done || state_ == State::Aborted; }
    const Result& result() const { return result_; }
    // Running: the time on the clock (0 until the rollout is done); after: the
    // run's total, or the time the abandoned run reached.
    uint32_t elapsedMs(uint32_t nowMs) const;
    float distanceM() const { return distance_; }
    float speedKmh() const { return speed_; }
    uint8_t splitsDone() const { return result_.count; }
    // Back to Idle, forgetting the result.
    void reset();

private:
    void arm(uint32_t nowMs);
    void launch(uint32_t startMs, uint32_t nowMs);
    void finish(bool aborted, uint32_t nowMs);
    void crossSplits(float vPrevMs, float vMs, float dPrev, uint32_t tPrev, uint32_t t);
    void addSplit(uint32_t atMs, float speedMs);

    Run run_ = Run::ZeroTo100;
    bool rollout_ = false;
    State state_ = State::Idle;
    Result result_;
    uint8_t seq_ = 0;
    // Speed samples.
    bool haveSample_ = false;
    float speed_ = 0;           // km/h, the last sample
    uint32_t sampleMs_ = 0;     // when
    uint32_t stillSinceMs_ = 0; // standing: below kArmKmh since; rolling: below the start since
    bool still_ = false;
    // Accelerometer.
    uint32_t launchGMs_ = 0;    // when forward g first exceeded kLaunchG while armed, 0 for not yet
    // The run.
    uint32_t t0_ = 0;           // the clock's zero
    bool clockSet_ = false;     // false until the rollout is done
    uint32_t launchMs_ = 0;     // when the car began to move (standing)
    uint8_t movingSamples_ = 0;
    float firstMoving_ = 0;     // km/h, the first sample above kArmKmh
    uint32_t firstMovingMs_ = 0;
    uint32_t lastStillMs_ = 0;  // the sample before it
    bool refined_ = false;      // the start has been placed (by the accelerometer or the slope)
    bool rampPending_ = false;  // the stretch from the start to the first moving sample is not yet integrated
    float distance_ = 0;        // metres from the launch
    float peak_ = 0;            // km/h, the highest sample this run
    uint32_t endMs_ = 0;        // Done or Aborted: when
};

}  // namespace motion
}  // namespace hg
