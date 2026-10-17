#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <string>

// Time of day on the hub (docs/time-of-day.md): set by NTP when the hub is on
// the home network, by the phone's clock from the page otherwise, and kept by
// a DS3231 on the carrier's I2C bus when one is fitted. The hub applies its
// time zone and puts the local time in every beacon for the gauges' clocks.
namespace time_service {

// After the Wi-Fi is up.
void begin();
// From the main loop: feeds the manager's beacon, writes the clock chip after a sync.
void poll(uint32_t nowMs);

// The clock of the device the page runs on. Taken unless NTP has already set
// the time; false then.
bool setFromDevice(uint32_t unixSeconds);
// A POSIX zone string such as "AEST-10" or "AEST-10AEDT,M10.1.0,M4.1.0/3",
// with a name for the page. False, with why, if it is not usable.
bool setZone(const char* posix, const char* name, std::string& error);

// The time as the gauges see it: seconds since 1970 in the hub's zone, or 0.
uint32_t localSeconds();

// {set, source, unix, local, tz, tz_name, rtc, synced_s}
void statusJson(JsonObject out);

}  // namespace time_service
