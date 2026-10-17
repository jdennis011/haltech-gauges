#include "ip_link.h"

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "ip_frames.h"

namespace ip_link {

namespace {

const char* kPrefs = "gauge";
const uint32_t kRetryMs = 10000;
const uint32_t kKeepaliveMs = 2000;

std::string ssid, password, host;
bool on = false;
int sock = -1;
sockaddr_in hubAddr = {};
uint32_t lastTryMs = 0;
uint32_t lastKeepaliveMs = 0;
bool wasConnected = false;
char statusText[64] = "off";

hg::CanFrame rxFrames[hg::kIpMaxFrames];
size_t rxCount = 0;
size_t rxNext = 0;

void load() {
    Preferences p;
    p.begin(kPrefs, true);
    ssid = p.getString("ip_ssid", "").c_str();
    password = p.getString("ip_pass", "").c_str();
    host = p.getString("ip_host", "192.168.4.1").c_str();
    p.end();
    on = !ssid.empty();
}

void save() {
    Preferences p;
    p.begin(kPrefs, false);
    p.putString("ip_ssid", ssid.c_str());
    p.putString("ip_pass", password.c_str());
    p.putString("ip_host", host.c_str());
    p.end();
}

void closeSocket() {
    if (sock >= 0) close(sock);
    sock = -1;
    rxCount = rxNext = 0;
}

bool openSocket() {
    closeSocket();
    hubAddr = {};
    hubAddr.sin_family = AF_INET;
    hubAddr.sin_port = htons(hg::kIpBusPort);
    if (inet_aton(host.c_str(), &hubAddr.sin_addr) == 0) {
        snprintf(statusText, sizeof(statusText), "bad hub address %s", host.c_str());
        return false;
    }
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        snprintf(statusText, sizeof(statusText), "no socket");
        return false;
    }
    const int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    snprintf(statusText, sizeof(statusText), "linked to %s as %s", host.c_str(), WiFi.localIP().toString().c_str());
    return true;
}

void sendPacket(const hg::CanFrame* frames, size_t count) {
    if (sock < 0) return;
    uint8_t packet[hg::kIpMaxPacket];
    const size_t len = hg::packIpFrames(frames, count, packet, sizeof(packet));
    if (len) sendto(sock, packet, len, 0, reinterpret_cast<sockaddr*>(&hubAddr), sizeof(hubAddr));
}

void join(uint32_t nowMs) {
    lastTryMs = nowMs;
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid.c_str(), password.c_str());
    snprintf(statusText, sizeof(statusText), "joining %s", ssid.c_str());
}

}  // namespace

void begin() {
    load();
    if (on) join(millis());
}

bool enabled() { return on; }

bool connected() { return on && sock >= 0; }

void set(const char* newSsid, const char* newPassword, const char* newHost) {
    ssid = newSsid ? newSsid : "";
    password = newPassword ? newPassword : "";
    if (newHost && newHost[0]) host = newHost;
    on = !ssid.empty();
    save();
    closeSocket();
    if (on) {
        join(millis());
    } else {
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
        snprintf(statusText, sizeof(statusText), "off");
    }
}

bool send(const hg::CanFrame& frame) {
    if (sock < 0) return false;
    sendPacket(&frame, 1);
    return true;
}

bool read(hg::CanFrame& out) {
    if (sock < 0) return false;
    if (rxNext >= rxCount) {
        uint8_t buf[hg::kIpMaxPacket + 16];
        const int n = recvfrom(sock, buf, sizeof(buf), MSG_DONTWAIT, nullptr, nullptr);
        if (n <= 0) return false;
        const int count = hg::unpackIpFrames(buf, size_t(n), rxFrames, hg::kIpMaxFrames);
        if (count <= 0) return false;
        rxCount = size_t(count);
        rxNext = 0;
    }
    out = rxFrames[rxNext++];
    return true;
}

void service(uint32_t nowMs) {
    if (!on) return;
    const bool up = WiFi.status() == WL_CONNECTED;
    if (!up) {
        if (wasConnected) {
            wasConnected = false;
            closeSocket();
            snprintf(statusText, sizeof(statusText), "lost %s", ssid.c_str());
        }
        if (uint32_t(nowMs - lastTryMs) >= kRetryMs) join(nowMs);
        return;
    }
    if (!wasConnected) {
        wasConnected = true;
        if (openSocket()) Serial.printf("ip link: %s\n", statusText);
    }
    // A keepalive, so the hub knows where this gauge is before it has anything to say.
    if (sock >= 0 && uint32_t(nowMs - lastKeepaliveMs) >= kKeepaliveMs) {
        lastKeepaliveMs = nowMs;
        sendPacket(nullptr, 0);
    }
}

const char* status() { return statusText; }

}  // namespace ip_link
