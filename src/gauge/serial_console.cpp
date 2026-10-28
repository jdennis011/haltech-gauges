#include "serial_console.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "audio.h"
#include "can_task.h"
#include "display.h"
#include "face_model.h"
#include "face_store.h"
#include "ip_link.h"
#include "proto.h"

namespace serial_console {

namespace {

const size_t kMaxLine = hg::cfg::kMaxConfigBytes + hg::cfg::kMaxImageMapBytes;

std::string line;
// A JSON document may span lines: it ends when its braces balance, or is
// given up when nothing more arrives for a while.
int depth = 0;
bool inString = false;
bool escaped = false;
bool json = false;
uint32_t lastByteMs = 0;
const uint32_t kJsonIdleMs = 2000;

void resetJson() {
    line.clear();
    json = false;
    depth = 0;
    inString = escaped = false;
}

void help() {
    Serial.println("Gauge console:");
    Serial.println("  {...}                 a config as JSON, shown without storing (may span lines)");
    Serial.println("  save {...}            a config as JSON, stored as the gauge's own");
    Serial.println("  end                   back to the stored config");
    Serial.println("  face N                show face N (from 1)");
    Serial.println("  bright N              brightness 0..255");
    Serial.println("  intro [style] [ms]    play the intro now: sweep, chase or fade; duration");
    Serial.println("  time HH:MM[:SS]       set the clock the clock widgets show");
    Serial.println("  tone [HZ] [MS]        a test tone on the speaker (default 1000 Hz, 300 ms)");
    Serial.println("  chime                 the identify chime");
    Serial.println("  play N                a built-in sound: 1 beep, 2 chime, 3 alarm, 4 riff");
    Serial.println("  volume N              speaker volume 0..100, kept");
    Serial.println("  info                  faces, hub, heap, audio");
    Serial.println("  link SSID PASSWORD [HUB]  reach the hub over Wi-Fi: its access point (hub 192.168.4.1), or a network with its address");
    Serial.println("  link                  the link's state;  link off  drops it");
}

void applyConfig(const char* text, size_t len, bool store) {
    uint8_t* buf = nullptr;
    const hg::proto::XferKind kind = store ? hg::proto::XferKind::Config : hg::proto::XferKind::SwitchConfig;
    if (face_store::accept(kind, uint32_t(len), &buf) != hg::proto::XferStatus::Ok || !buf) {
        Serial.println("config refused: too large or no memory");
        return;
    }
    memcpy(buf, text, len);
    if (face_store::complete(kind, buf, uint32_t(len))) {
        Serial.printf("config applied%s: %u bytes\n", store ? " and stored" : "", unsigned(len));
    }
    // complete() prints the parser's reason when it refuses.
}

void run(const std::string& cmd) {
    if (cmd.empty()) return;
    if (cmd[0] == '{') {
        applyConfig(cmd.data(), cmd.size(), false);
        return;
    }
    if (cmd.rfind("save ", 0) == 0) {
        const size_t at = cmd.find('{');
        if (at == std::string::npos) {
            Serial.println("save: give the config as JSON after it");
            return;
        }
        applyConfig(cmd.data() + at, cmd.size() - at, true);
        return;
    }
    if (cmd == "?" || cmd == "help") {
        help();
    } else if (cmd == "end") {
        face_store::endTry();
        Serial.println("back to the stored config");
    } else if (cmd.rfind("face ", 0) == 0) {
        const int n = atoi(cmd.c_str() + 5);
        can_task::benchFace(n - 1);
        Serial.printf("face %d\n", n);
    } else if (cmd.rfind("bright ", 0) == 0) {
        const int n = atoi(cmd.c_str() + 7);
        can_task::benchBrightness(n < 0 ? 0 : (n > 255 ? 255 : n));
        Serial.printf("brightness %d\n", n);
    } else if (cmd == "intro" || cmd.rfind("intro ", 0) == 0) {
        hg::proto::Intro i;
        i.force = true;
        const char* rest = cmd.c_str() + 5;
        while (*rest == ' ') rest++;
        if (strncmp(rest, "chase", 5) == 0) i.style = hg::proto::IntroStyle::Chase;
        if (strncmp(rest, "fade", 4) == 0) i.style = hg::proto::IntroStyle::Fade;
        const char* sp = strchr(rest, ' ');
        if (sp && atoi(sp) >= 200) i.durationMs = uint16_t(atoi(sp));
        can_task::benchIntro(i);
        Serial.printf("intro %s %u ms\n", i.style == hg::proto::IntroStyle::Chase ? "chase" : i.style == hg::proto::IntroStyle::Fade ? "fade" : "sweep",
                      unsigned(i.durationMs));
    } else if (cmd.rfind("time ", 0) == 0) {
        int h = 0, m = 0, s = 0;
        if (sscanf(cmd.c_str() + 5, "%d:%d:%d", &h, &m, &s) < 2 || h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) {
            Serial.println("time: HH:MM or HH:MM:SS");
            return;
        }
        can_task::benchLocalTime(uint32_t(h) * 3600u + uint32_t(m) * 60u + uint32_t(s));
        Serial.printf("clock set to %02d:%02d:%02d\n", h, m, s);
    } else if (cmd == "link") {
        Serial.printf("link: %s\n", ip_link::status());
    } else if (cmd == "link off") {
        ip_link::set("", "", "");
        Serial.println("link off");
    } else if (cmd.rfind("link ", 0) == 0) {
        char ssid[33] = "", pass[65] = "", host[32] = "";
        const int n = sscanf(cmd.c_str() + 5, "%32s %64s %31s", ssid, pass, host);
        if (n < 1) {
            Serial.println("link: SSID PASSWORD [HUB]");
            return;
        }
        ip_link::set(ssid, pass, host);
        Serial.printf("link: joining %s, hub %s\n", ssid, host[0] ? host : "192.168.4.1");
    } else if (cmd == "tone" || cmd.rfind("tone ", 0) == 0) {
        int hz = 1000, ms = 300;
        sscanf(cmd.c_str() + 4, "%d %d", &hz, &ms);
        if (!audio::present()) {
            Serial.println("tone: no codec on this board");
            return;
        }
        audio::tone(uint16_t(hz < 0 ? 0 : hz), uint16_t(ms < 0 ? 0 : ms));
        Serial.printf("tone %d Hz for %d ms at volume %u\n", hz, ms, unsigned(audio::volume()));
    } else if (cmd.rfind("play ", 0) == 0) {
        const int n = atoi(cmd.c_str() + 5);
        audio::play(uint8_t(n < 0 ? 0 : n));
        Serial.println(audio::present() ? "playing" : "play: no codec on this board");
    } else if (cmd == "chime") {
        audio::chime();
        Serial.println(audio::present() ? "chime" : "chime: no codec on this board");
    } else if (cmd.rfind("volume ", 0) == 0) {
        const int n = atoi(cmd.c_str() + 7);
        audio::setVolume(uint8_t(n < 0 ? 0 : (n > 100 ? 100 : n)));
        Serial.printf("volume %u\n", unsigned(audio::volume()));
    } else if (cmd == "info") {
        Serial.printf("faces %u, on face %u, hub %d, trying %d, heap %u, audio %s\n", unsigned(face_store::faceCount()),
                      unsigned(face_store::activeFace() + 1), int(can_task::hubPresent()), int(face_store::trying()),
                      unsigned(ESP.getFreeHeap()), audio::present() ? "codec" : "none");
    } else {
        Serial.printf("unknown: %s (? for help)\n", cmd.c_str());
    }
}

}  // namespace

void begin() { line.reserve(1024); }

void poll() {
    if (json && !line.empty() && millis() - lastByteMs >= kJsonIdleMs) {
        Serial.printf("config incomplete: %u bytes arrived, braces not balanced; dropped\n", unsigned(line.size()));
        resetJson();
    }
    while (Serial.available()) {
        const char c = char(Serial.read());
        lastByteMs = millis();
        if (!json && line.empty() && (c == '\r' || c == '\n' || c == ' ')) continue;
        if (!json && line.empty() && c == '{') json = true;
        if (!json && line.size() == 5 && line == "save " && c == '{') json = true;
        if (json) {
            // Track brace depth outside strings, so a pasted file's newlines pass through.
            if (escaped) {
                escaped = false;
            } else if (inString) {
                if (c == '\\') escaped = true;
                else if (c == '"') inString = false;
            } else if (c == '"') {
                inString = true;
            } else if (c == '{') {
                depth++;
            } else if (c == '}') {
                depth--;
            }
            if (c != '\r') line.push_back(c);
            if (depth <= 0 && c == '}') {
                run(line);
                resetJson();
            } else if (line.size() >= kMaxLine) {
                Serial.println("config refused: too large");
                resetJson();
            }
            continue;
        }
        if (c == '\n' || c == '\r') {
            run(line);
            line.clear();
        } else if (line.size() < 256) {
            line.push_back(c);
        }
    }
}

}  // namespace serial_console
