#include "motion.h"

#include <math.h>

namespace hg {
namespace motion {

float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 cross(const Vec3& a, const Vec3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
Vec3 sub(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 scale(const Vec3& a, float k) { return {a.x * k, a.y * k, a.z * k}; }
float length(const Vec3& a) { return sqrtf(dot(a, a)); }
Vec3 normalize(const Vec3& a) {
    const float len = length(a);
    return len > 1e-6f ? scale(a, 1.0f / len) : a;
}

namespace {

// The part of `v` at right angles to the unit vector `axis`.
Vec3 flatten(const Vec3& v, const Vec3& axis) { return sub(v, scale(axis, dot(v, axis))); }

// Any unit vector at right angles to `axis`, for when the guess lies along it.
Vec3 anyPerpendicular(const Vec3& axis) {
    const Vec3 trial = fabsf(axis.x) < 0.9f ? Vec3{1, 0, 0} : Vec3{0, 1, 0};
    return normalize(flatten(trial, axis));
}

}  // namespace

bool setLevel(Orientation& o, const Vec3& resting, const Vec3& defaultForward) {
    const float len = length(resting);
    if (len < 0.7f || len > 1.3f) return false;
    o.up = scale(resting, 1.0f / len);
    o.haveLevel = true;
    // Keep a captured forward, re-squared to the new up; otherwise the board's guess.
    const Vec3 keep = o.haveForward ? o.forward : defaultForward;
    Vec3 flat = flatten(keep, o.up);
    if (length(flat) < 0.05f) {
        flat = anyPerpendicular(o.up);
        o.haveForward = false;
    }
    o.forward = normalize(flat);
    return true;
}

bool setForward(Orientation& o, const Vec3& braking, float minG) {
    if (!o.haveLevel) return false;
    const Vec3 flat = flatten(braking, o.up);
    if (length(flat) < minG) return false;
    o.forward = normalize(scale(flat, -1));
    o.haveForward = true;
    return true;
}

CarAccel resolve(const Orientation& o, const Vec3& reading) {
    CarAccel a;
    a.lat = dot(reading, o.right());
    a.lon = dot(reading, o.forward);
    a.vert = dot(reading, o.up) - 1.0f;
    return a;
}

float LowPass::step(float x, float dtSeconds) {
    if (!primed_) {
        primed_ = true;
        y_ = x;
        return y_;
    }
    if (dtSeconds <= 0) return y_;
    const float rc = 1.0f / (2.0f * 3.14159265f * cutoff_);
    const float k = dtSeconds / (rc + dtSeconds);
    y_ += k * (x - y_);
    return y_;
}

}  // namespace motion
}  // namespace hg
