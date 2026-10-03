#pragma once

#include <ArduinoJson.h>

// The hub's Wi-Fi: an access point that is always on, plus an optional
// connection to a home network (station mode) saved in flash.
namespace wifi_ap {

void begin();
// Answers captive-portal DNS. Call from loop().
void loop();
// Saves and applies station credentials. An empty SSID forgets them.
void setStation(const char* ssid, const char* password);
// Adds "ap", "sta" and "hostname" to a JSON object.
void statusJson(JsonObject out);

}  // namespace wifi_ap
