#include "imu.h"

#include <Arduino.h>
#include <Preferences.h>
#include <SensorQMI8658.hpp>
#include <Wire.h>
#include <stdio.h>

#include "board_pins.h"

using hg::motion::CarAccel;
using hg::motion::Orientation;
using hg::motion::Vec3;

namespace imu {

namespace {

const char* kPrefs = "motion";
const uint32_t kPeriodMs = 20;
// The sensor axis expected to point forward until Set forward says: the
// display faces the driver, so its normal points backward.
const Vec3 kDefaultForward{0, 0, -1};

SensorQMI8658 sensor;
bool started = false;
Orientation orientation;
Reading latest;
float lonLast = 0;
uint32_t lastReadMs = 0;
uint32_t busyUs = 0;
hg::motion::LowPass latLp(5), lonLp(5), vertLp(5);

void load() {
    Preferences p;
    p.begin(kPrefs, true);
    orientation.haveLevel = p.getBool("lvl", false);
    orientation.haveForward = p.getBool("fwd", false);
    orientation.up = {p.getFloat("upx", 0), p.getFloat("upy", 0), p.getFloat("upz", 1)};
    orientation.forward = {p.getFloat("fwx", 1), p.getFloat("fwy", 0), p.getFloat("fwz", 0)};
    p.end();
    if (!orientation.haveLevel) {
        orientation = Orientation();
        hg::motion::setLevel(orientation, {0, 0, 1}, kDefaultForward);
        orientation.haveLevel = false;
    }
}

void save() {
    Preferences p;
    p.begin(kPrefs, false);
    p.putBool("lvl", orientation.haveLevel);
    p.putBool("fwd", orientation.haveForward);
    p.putFloat("upx", orientation.up.x);
    p.putFloat("upy", orientation.up.y);
    p.putFloat("upz", orientation.up.z);
    p.putFloat("fwx", orientation.forward.x);
    p.putFloat("fwy", orientation.forward.y);
    p.putFloat("fwz", orientation.forward.z);
    p.end();
}

bool read(Vec3& out) {
    float x, y, z;
    if (!started || !sensor.getAccelerometer(x, y, z)) return false;
    out = {x, y, z};
    return true;
}

// The mean of a few readings over about 100 ms.
bool average(Vec3& out) {
    Vec3 sum;
    int n = 0;
    for (int i = 0; i < 10; i++) {
        Vec3 v;
        if (read(v)) {
            sum.x += v.x;
            sum.y += v.y;
            sum.z += v.z;
            n++;
        }
        delay(10);
    }
    if (!n) return false;
    out = hg::motion::scale(sum, 1.0f / float(n));
    return true;
}

}  // namespace

void begin() {
    load();
    started = sensor.begin(Wire, QMI8658_L_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL);
    if (!started) {
        Serial.println("IMU: no QMI8658 found");
        return;
    }
    sensor.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_125Hz, SensorQMI8658::LPF_MODE_3);
    sensor.enableAccelerometer();
    Serial.printf("IMU: QMI8658 id 0x%02X, level %s, forward %s\n", sensor.getChipID(),
                  orientation.haveLevel ? "set" : "not set", orientation.haveForward ? "set" : "not set");
}

bool present() { return started; }

void poll(uint32_t nowMs) {
    if (!started || uint32_t(nowMs - lastReadMs) < kPeriodMs) return;
    const float dt = lastReadMs ? float(nowMs - lastReadMs) / 1000.0f : 0.02f;
    lastReadMs = nowMs;
    Vec3 v;
    const uint32_t t0 = micros();
    const bool ok = read(v);
    busyUs += micros() - t0;
    if (!ok) return;
    const CarAccel a = hg::motion::resolve(orientation, v);
    lonLast = a.lon;
    latest.valid = true;
    latest.lat = latLp.step(a.lat, dt);
    latest.lon = lonLp.step(a.lon, dt);
    latest.vert = vertLp.step(a.vert, dt);
}

Reading reading() { return latest; }
uint32_t msPerSecond() {
    const uint32_t ms = busyUs / 1000;
    busyUs = 0;
    return ms;
}
float lonRaw() { return lonLast; }

bool calibrated() { return orientation.haveLevel && orientation.haveForward; }
bool levelSet() { return orientation.haveLevel; }
bool forwardSet() { return orientation.haveForward; }

bool setLevel() {
    Vec3 v;
    if (!average(v)) return false;
    if (!hg::motion::setLevel(orientation, v, kDefaultForward)) {
        Serial.printf("IMU: level not set, the reading is %.2f g\n", hg::motion::length(v));
        return false;
    }
    save();
    Serial.printf("IMU: level set, up (%.2f, %.2f, %.2f)\n", orientation.up.x, orientation.up.y, orientation.up.z);
    return true;
}

bool setForward() {
    Vec3 v;
    if (!average(v)) return false;
    if (!hg::motion::setForward(orientation, v)) {
        Serial.println("IMU: forward not set: brake harder, or set level first");
        return false;
    }
    save();
    Serial.printf("IMU: forward set (%.2f, %.2f, %.2f)\n", orientation.forward.x, orientation.forward.y, orientation.forward.z);
    return true;
}

void clearCalibration() {
    orientation = Orientation();
    hg::motion::setLevel(orientation, {0, 0, 1}, kDefaultForward);
    orientation.haveLevel = false;
    orientation.haveForward = false;
    save();
}

void status(char* out, size_t len) {
    if (!started) {
        snprintf(out, len, "imu: none");
        return;
    }
    Vec3 v;
    const bool ok = read(v);
    snprintf(out, len, "imu: raw (%.2f, %.2f, %.2f) g, lat %.2f lon %.2f vert %.2f, level %s, forward %s", ok ? v.x : 0,
             ok ? v.y : 0, ok ? v.z : 0, latest.lat, latest.lon, latest.vert, orientation.haveLevel ? "set" : "not set",
             orientation.haveForward ? "set" : "not set");
}

}  // namespace imu
