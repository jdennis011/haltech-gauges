#include "wifi_ap.h"

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>

#include "hub_config.h"

namespace wifi_ap {

namespace {

DNSServer dns;
const IPAddress kApIp(192, 168, 4, 1);
const IPAddress kApMask(255, 255, 255, 0);
String staSsid;
String staPassword;

void applyTxPower() {
#if CONFIG_IDF_TARGET_ESP32C3
    // The small C3 boards (Lolin C3 Mini, SuperMini) drop connections at full
    // transmit power; Lolin's own advice is to back it off.
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
}

void startStation() {
    if (staSsid.isEmpty()) return;
    WiFi.begin(staSsid.c_str(), staPassword.c_str());
    applyTxPower();
}

}  // namespace

void begin() {
    Preferences prefs;
    prefs.begin("hub", true);
    staSsid = prefs.getString("sta_ssid", "");
    staPassword = prefs.getString("sta_pass", "");
    prefs.end();

    WiFi.persistent(false);
    // Modem power save makes a station answer only at beacon intervals: pings
    // of hundreds of ms, lost packets and stalled downloads. The hub is on
    // switched 12 V, so it stays awake.
    WiFi.setSleep(false);
    WiFi.setHostname(HUB_HOSTNAME);
    WiFi.mode(staSsid.isEmpty() ? WIFI_AP : WIFI_AP_STA);
    WiFi.softAP(HUB_AP_SSID, HUB_AP_PASSWORD, 1, 0, 4);
    WiFi.softAPConfig(kApIp, kApIp, kApMask);
    applyTxPower();
    WiFi.setAutoReconnect(true);
    startStation();

    // Any name typed into a browser on the access point lands on the hub.
    dns.start(53, "*", kApIp);
    if (MDNS.begin(HUB_HOSTNAME)) MDNS.addService("http", "tcp", 80);
}

void loop() { dns.processNextRequest(); }

void setStation(const char* ssid, const char* password) {
    staSsid = ssid ? ssid : "";
    staPassword = password ? password : "";
    Preferences prefs;
    prefs.begin("hub", false);
    prefs.putString("sta_ssid", staSsid);
    prefs.putString("sta_pass", staPassword);
    prefs.end();

    WiFi.disconnect(false, false);
    if (staSsid.isEmpty()) {
        WiFi.mode(WIFI_AP);
        return;
    }
    WiFi.mode(WIFI_AP_STA);
    startStation();
}

void statusJson(JsonObject out) {
    const bool connected = WiFi.status() == WL_CONNECTED;
    JsonObject ap = out["ap"].to<JsonObject>();
    ap["ssid"] = HUB_AP_SSID;
    ap["password"] = HUB_AP_PASSWORD;
    ap["ip"] = WiFi.softAPIP().toString();
    ap["clients"] = WiFi.softAPgetStationNum();
    JsonObject sta = out["sta"].to<JsonObject>();
    sta["ssid"] = staSsid;
    sta["connected"] = connected;
    sta["ip"] = connected ? WiFi.localIP().toString() : String("");
    sta["rssi"] = connected ? WiFi.RSSI() : 0;
    out["hostname"] = HUB_HOSTNAME;
}

}  // namespace wifi_ap
