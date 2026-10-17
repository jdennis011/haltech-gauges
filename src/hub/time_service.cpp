#include "time_service.h"

#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>
#include <esp_sntp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <atomic>

#include "hub_state.h"

namespace time_service {

namespace {

const char* kPrefs = "hub";
const char* kDefaultZone = "AEST-10";
const char* kDefaultZoneName = "Australia/Brisbane";
const uint8_t kRtcAddress = 0x68;
const uint32_t kRtcWriteEveryMs = 24u * 60u * 60u * 1000u;
const time_t kPlausible = 1700000000;  // 2023: anything earlier is no time at all

// The carrier's I2C bus for the clock chip (electrical-spec.md).
#if CONFIG_IDF_TARGET_ESP32P4
const int kSda = 7, kScl = 8;
#elif CONFIG_IDF_TARGET_ESP32C6
const int kSda = 6, kScl = 7;
#else
const int kSda = -1, kScl = -1;
#endif

enum class Source : uint8_t { None, Rtc, Device, Ntp };

const char* sourceName(Source s) {
    switch (s) {
        case Source::None: return "none";
        case Source::Rtc: return "clock chip";
        case Source::Device: return "phone or PC";
        case Source::Ntp: return "ntp";
    }
    return "none";
}

std::string zone, zoneName;
std::atomic<uint8_t> source{uint8_t(Source::None)};
std::atomic<bool> ntpEvent{false};  // set from the SNTP callback
uint32_t syncedMs = 0;
bool everSynced = false;
bool rtcPresent = false;
bool rtcGood = false;  // its oscillator has not stopped since it was last set
uint32_t rtcWrittenMs = 0;
bool rtcWritten = false;
uint32_t lastPollMs = 0;

uint8_t bcd(uint8_t v) { return uint8_t(((v / 10) << 4) | (v % 10)); }
uint8_t unbcd(uint8_t v) { return uint8_t((v >> 4) * 10 + (v & 0x0F)); }

// Days since 1970 for a civil date (proleptic Gregorian), from Howard Hinnant.
int64_t daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = unsigned(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return int64_t(era) * 146097 + int64_t(doe) - 719468;
}

bool rtcRead(time_t& out) {
    Wire.beginTransmission(kRtcAddress);
    Wire.write(0x00);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(int(kRtcAddress), 7) != 7) return false;
    uint8_t r[7];
    for (int i = 0; i < 7; i++) r[i] = uint8_t(Wire.read());
    const int sec = unbcd(r[0] & 0x7F), min = unbcd(r[1] & 0x7F);
    const int hour = (r[2] & 0x40) ? (unbcd(r[2] & 0x1F) % 12 + ((r[2] & 0x20) ? 12 : 0)) : unbcd(r[2] & 0x3F);
    const int day = unbcd(r[4] & 0x3F), month = unbcd(r[5] & 0x1F);
    const int year = 2000 + ((r[5] & 0x80) ? 100 : 0) + unbcd(r[6]);
    if (month < 1 || month > 12 || day < 1 || day > 31) return false;
    out = time_t(daysFromCivil(year, month, day) * 86400 + hour * 3600 + min * 60 + sec);
    return true;
}

bool rtcStopped() {
    Wire.beginTransmission(kRtcAddress);
    Wire.write(0x0F);
    if (Wire.endTransmission(false) != 0) return true;
    if (Wire.requestFrom(int(kRtcAddress), 1) != 1) return true;
    return (Wire.read() & 0x80) != 0;  // OSF: the oscillator stopped, so the time is not to be trusted
}

bool rtcWrite(time_t now) {
    struct tm g;
    gmtime_r(&now, &g);
    Wire.beginTransmission(kRtcAddress);
    Wire.write(0x00);
    Wire.write(bcd(uint8_t(g.tm_sec)));
    Wire.write(bcd(uint8_t(g.tm_min)));
    Wire.write(bcd(uint8_t(g.tm_hour)));  // 24-hour mode
    Wire.write(bcd(uint8_t(g.tm_wday + 1)));
    Wire.write(bcd(uint8_t(g.tm_mday)));
    Wire.write(uint8_t(bcd(uint8_t(g.tm_mon + 1)) | (g.tm_year + 1900 >= 2100 ? 0x80 : 0)));
    Wire.write(bcd(uint8_t((g.tm_year + 1900) % 100)));
    if (Wire.endTransmission() != 0) return false;
    // Clear the oscillator-stop flag: the time is good from here.
    Wire.beginTransmission(kRtcAddress);
    Wire.write(0x0F);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(int(kRtcAddress), 1) != 1) return false;
    const uint8_t status = uint8_t(Wire.read());
    Wire.beginTransmission(kRtcAddress);
    Wire.write(0x0F);
    Wire.write(uint8_t(status & 0x7F));
    return Wire.endTransmission() == 0;
}

void applyZone() {
    setenv("TZ", zone.c_str(), 1);
    tzset();
}

void onNtpSync(struct timeval*) { ntpEvent = true; }

void setSystemTime(time_t t) {
    struct timeval tv = {t, 0};
    settimeofday(&tv, nullptr);
}

bool haveTime() { return time(nullptr) > kPlausible; }

// Seconds between local time and UTC right now, from the zone in force.
int32_t utcOffset(time_t now) {
    struct tm lt, gt;
    localtime_r(&now, &lt);
    gmtime_r(&now, &gt);
    int32_t days = lt.tm_yday - gt.tm_yday;
    if (lt.tm_year != gt.tm_year) days = lt.tm_year > gt.tm_year ? 1 : -1;
    return days * 86400 + (lt.tm_hour - gt.tm_hour) * 3600 + (lt.tm_min - gt.tm_min) * 60;
}

}  // namespace

void begin() {
    Preferences p;
    p.begin(kPrefs, true);
    zone = p.getString("tz", kDefaultZone).c_str();
    zoneName = p.getString("tz_name", kDefaultZoneName).c_str();
    p.end();
    applyZone();

    if (kSda >= 0) {
        Wire.begin(kSda, kScl);
        Wire.beginTransmission(kRtcAddress);
        rtcPresent = Wire.endTransmission() == 0;
        if (rtcPresent) {
            rtcGood = !rtcStopped();
            time_t t;
            if (rtcGood && rtcRead(t) && t > kPlausible) {
                setSystemTime(t);
                source = uint8_t(Source::Rtc);
                Serial.println("Time from the clock chip");
            } else {
                Serial.println("Clock chip present but not set");
            }
        }
    }

    // NTP whenever the home network is reachable; the callback says when.
    esp_sntp_set_time_sync_notification_cb(onNtpSync);
    configTzTime(zone.c_str(), "pool.ntp.org", "time.google.com", "time.cloudflare.com");
}

void poll(uint32_t nowMs) {
    if (ntpEvent.exchange(false)) {
        source = uint8_t(Source::Ntp);
        syncedMs = nowMs;
        everSynced = true;
        if (rtcPresent && (!rtcWritten || uint32_t(nowMs - rtcWrittenMs) >= kRtcWriteEveryMs)) {
            if (rtcWrite(time(nullptr))) {
                rtcWritten = true;
                rtcGood = true;
                rtcWrittenMs = nowMs;
                Serial.println("Clock chip set from NTP");
            }
        }
        if (!rtcWritten) {
            Serial.println("Time from NTP");
            rtcWritten = !rtcPresent;  // nothing to write to
        }
    }
    if (uint32_t(nowMs - lastPollMs) < 1000) return;
    lastPollMs = nowMs;
    const uint32_t local = localSeconds();
    hub::Lock lock;
    hub::manager().setLocalTime(local);
}

bool setFromDevice(uint32_t unixSeconds) {
    if (Source(source.load()) == Source::Ntp) return false;
    if (unixSeconds < uint32_t(kPlausible)) return false;
    setSystemTime(time_t(unixSeconds));
    source = uint8_t(Source::Device);
    syncedMs = millis();
    everSynced = true;
    if (rtcPresent && rtcWrite(time(nullptr))) {
        rtcGood = true;
        rtcWritten = true;
        rtcWrittenMs = millis();
    }
    Serial.println("Time from the page's device");
    return true;
}

bool setZone(const char* posix, const char* name, std::string& error) {
    if (!posix || !posix[0] || strlen(posix) > 48) {
        error = "the zone is a POSIX TZ string of up to 48 characters, such as AEST-10";
        return false;
    }
    for (const char* c = posix; *c; c++) {
        if (*c < 0x20 || *c > 0x7E || *c == '"') {
            error = "the zone has a character that is not allowed";
            return false;
        }
    }
    zone = posix;
    zoneName = name && name[0] ? name : posix;
    if (zoneName.size() > 48) zoneName.resize(48);
    applyZone();
    Preferences p;
    p.begin(kPrefs, false);
    p.putString("tz", zone.c_str());
    p.putString("tz_name", zoneName.c_str());
    p.end();
    return true;
}

uint32_t localSeconds() {
    if (!haveTime()) return 0;
    const time_t now = time(nullptr);
    return uint32_t(int64_t(now) + utcOffset(now));
}

void statusJson(JsonObject out) {
    const bool set = haveTime();
    out["set"] = set;
    out["source"] = sourceName(Source(source.load()));
    const time_t now = time(nullptr);
    out["unix"] = set ? uint32_t(now) : 0;
    char buf[32] = "";
    if (set) {
        struct tm lt;
        localtime_r(&now, &lt);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &lt);
    }
    out["local"] = buf;
    out["tz"] = zone;
    out["tz_name"] = zoneName;
    out["rtc"] = rtcPresent;
    out["rtc_ok"] = rtcPresent && rtcGood;
    out["synced_s"] = everSynced ? int((millis() - syncedMs) / 1000) : -1;
}

}  // namespace time_service
