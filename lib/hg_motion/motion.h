#pragma once

#include <stdint.h>

// The gauge's own accelerometer as a G meter (docs/motion.md): how the sensor
// sits in the car, learnt from two captures, and the maths that turns a raw
// reading into lateral, longitudinal and vertical g. No hardware here, so the
// host tests cover it.
namespace hg {
namespace motion {

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

float dot(const Vec3& a, const Vec3& b);
Vec3 cross(const Vec3& a, const Vec3& b);
Vec3 sub(const Vec3& a, const Vec3& b);
Vec3 scale(const Vec3& a, float k);
float length(const Vec3& a);
// A unit vector along `a`; `a` itself when it has no length.
Vec3 normalize(const Vec3& a);

// Where the car's axes point in the sensor's frame. "Set level" with the car
// at rest gives up (the resting reading points up: the sensor measures the
// reaction to gravity). "Set forward" while braking hard on a straight road
// gives forward: the horizontal part of that reading points backward. Any
// mounting angle works once both are known; with only the first, forward is
// the board's own guess projected flat.
struct Orientation {
    bool haveLevel = false;
    bool haveForward = false;
    Vec3 up{0, 0, 1};
    Vec3 forward{1, 0, 0};  // perpendicular to up
    Vec3 right() const { return cross(forward, up); }
};

// The resting reading in g. False, and nothing changed, when it is not about
// 1 g long (the car was moving, or the sensor is wrong). `defaultForward` is
// the sensor axis the board expects to point forward, used until Set forward.
bool setLevel(Orientation& o, const Vec3& resting, const Vec3& defaultForward);
// A reading taken while braking, in g. False, and nothing changed, when its
// horizontal part is shorter than `minG` (not braking hard enough) or there
// is no level yet.
bool setForward(Orientation& o, const Vec3& braking, float minG = 0.15f);

// A reading resolved into the car's axes, in g. lat is positive to the right
// (the car accelerating into a right turn), lon positive forward (accelerating,
// negative braking), vert positive upward beyond gravity.
struct CarAccel {
    float lat = 0, lon = 0, vert = 0;
};
CarAccel resolve(const Orientation& o, const Vec3& reading);

// A first-order low-pass filter stepped with the time since the last sample.
class LowPass {
public:
    explicit LowPass(float cutoffHz = 5) : cutoff_(cutoffHz) {}
    float step(float x, float dtSeconds);
    float value() const { return y_; }
    void reset() { primed_ = false; y_ = 0; }

private:
    float cutoff_;
    float y_ = 0;
    bool primed_ = false;
};

}  // namespace motion
}  // namespace hg
