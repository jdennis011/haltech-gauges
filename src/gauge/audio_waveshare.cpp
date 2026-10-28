#include "audio.h"

#include <Arduino.h>
#include <ESP_I2S.h>
#include <Preferences.h>
#include <Wire.h>
#include <math.h>
#include <string.h>

#include "board_pins.h"

// ES8311 bring-up follows Espressif's es8311 driver (the sequence Waveshare's
// 08_ES8311 demo uses), reduced to the playback path: 16 kHz, 16-bit I2S,
// codec as the serial slave, MCLK = 256 fs from the MCLK pin. The register
// writes go through Wire, which touch and the PMIC already share.
namespace audio {

namespace {

const uint8_t kCodecAddress = 0x18;
const uint32_t kSampleRate = 16000;
const uint32_t kFramesPerBlock = 128;
// How long the amplifier stays on after the last note, so a run of notes
// does not click it on and off between them.
const uint32_t kAmpHoldMs = 300;
const uint8_t kDefaultVolume = 60;
const char* kPrefs = "audio";
const char* kVolumeKey = "vol";

struct Note {
    uint16_t hz;
    uint16_t ms;
};

I2SClass i2s;
QueueHandle_t queue = nullptr;
bool codecOk = false;
volatile uint8_t volumePercent = kDefaultVolume;
volatile bool volumeDirty = false;

bool writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(kCodecAddress);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool readReg(uint8_t reg, uint8_t& value) {
    Wire.beginTransmission(kCodecAddress);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(kCodecAddress, uint8_t(1)) != 1) return false;
    value = uint8_t(Wire.read());
    return true;
}

// Keeps the bits in `keep`, sets the bits in `set`.
bool updateReg(uint8_t reg, uint8_t keep, uint8_t set) {
    uint8_t v = 0;
    if (!readReg(reg, v)) return false;
    return writeReg(reg, uint8_t((v & keep) | set));
}

bool applyVolume() {
    // DAC volume register: 0 is mute, 255 is 0 dB in 0.5 dB steps.
    const uint8_t reg = volumePercent == 0 ? 0 : uint8_t(uint16_t(volumePercent) * 256 / 100 - 1);
    return writeReg(0x32, reg);
}

bool codecInit() {
    bool ok = writeReg(0x00, 0x1F);  // reset everything
    delay(20);
    ok &= writeReg(0x00, 0x00);
    ok &= writeReg(0x00, 0x80);  // power on
    // Clocks: all on, MCLK from the MCLK pin, neither MCLK nor BCLK inverted.
    ok &= writeReg(0x01, 0x3F);
    ok &= updateReg(0x06, uint8_t(~0x20), 0x00);
    // Dividers for a 4.096 MHz MCLK at 16 kHz (the driver's coefficient
    // table): pre-divider 1, multiplier 1x, ADC and DAC dividers 1, single
    // speed, oversampling 16, LRCK = MCLK / 256, BCLK = MCLK / 4.
    ok &= updateReg(0x02, 0x07, 0x00);
    ok &= writeReg(0x03, 0x10);
    ok &= writeReg(0x04, 0x10);
    ok &= writeReg(0x05, 0x00);
    ok &= updateReg(0x06, 0xE0, 0x03);
    ok &= updateReg(0x07, 0xC0, 0x00);
    ok &= writeReg(0x08, 0xFF);
    // Serial port: slave, I2S, 16-bit in and out.
    ok &= updateReg(0x00, 0xBF, 0x00);
    ok &= writeReg(0x09, 0x0C);
    ok &= writeReg(0x0A, 0x0C);
    // Analog section, DAC and the output driver on; equalisers bypassed.
    ok &= writeReg(0x0D, 0x01);
    ok &= writeReg(0x0E, 0x02);
    ok &= writeReg(0x12, 0x00);
    ok &= writeReg(0x13, 0x10);
    ok &= writeReg(0x1C, 0x6A);
    ok &= writeReg(0x37, 0x08);
    ok &= applyVolume();
    return ok;
}

void writeSilence(uint32_t ms) {
    static int16_t zeros[kFramesPerBlock * 2] = {};
    uint32_t frames = kSampleRate * ms / 1000;
    while (frames) {
        const uint32_t n = frames < kFramesPerBlock ? frames : kFramesPerBlock;
        i2s.write(reinterpret_cast<uint8_t*>(zeros), n * 4);
        frames -= n;
    }
}

void playerTask(void*) {
    static int16_t block[kFramesPerBlock * 2];
    bool ampOn = false;
    float phase = 0;
    for (;;) {
        Note note;
        if (xQueueReceive(queue, &note, pdMS_TO_TICKS(kAmpHoldMs)) != pdTRUE) {
            if (ampOn) {
                digitalWrite(PIN_AUDIO_AMP, LOW);
                ampOn = false;
            }
            continue;
        }
        if (!ampOn) {
            digitalWrite(PIN_AUDIO_AMP, HIGH);
            ampOn = true;
            writeSilence(30);  // the amplifier wakes on zeros, not on the note's first edge
        }
        const uint32_t total = kSampleRate * note.ms / 1000;
        const uint32_t ramp = kSampleRate * 5 / 1000;  // 5 ms in and out, against clicks
        const float step = 2.0f * float(M_PI) * float(note.hz) / float(kSampleRate);
        uint32_t done = 0;
        while (done < total) {
            const uint32_t n = (total - done) < kFramesPerBlock ? (total - done) : kFramesPerBlock;
            for (uint32_t i = 0; i < n; i++) {
                const uint32_t at = done + i;
                float env = 1.0f;
                if (at < ramp) env = float(at) / float(ramp);
                else if (total - at <= ramp) env = float(total - at) / float(ramp);
                if (note.hz == 0) env = 0.0f;
                const int16_t s = int16_t(sinf(phase) * env * 12000.0f);
                phase += step;
                if (phase > 2.0f * float(M_PI)) phase -= 2.0f * float(M_PI);
                block[2 * i] = s;
                block[2 * i + 1] = s;
            }
            i2s.write(reinterpret_cast<uint8_t*>(block), n * 4);
            done += n;
        }
        if (uxQueueMessagesWaiting(queue) == 0) writeSilence(20);
    }
}

void enqueue(uint16_t hz, uint16_t ms) {
    if (!queue) return;
    Note note = {hz, ms};
    xQueueSend(queue, &note, 0);
}

}  // namespace

void begin() {
    pinMode(PIN_AUDIO_AMP, OUTPUT);
    digitalWrite(PIN_AUDIO_AMP, LOW);

    Wire.beginTransmission(kCodecAddress);
    if (Wire.endTransmission() != 0) {
        Serial.println("Audio: no ES8311 codec on the I2C bus");
        return;
    }

    Preferences prefs;
    if (prefs.begin(kPrefs, true)) {
        volumePercent = uint8_t(prefs.getUChar(kVolumeKey, kDefaultVolume));
        prefs.end();
    }
    if (volumePercent > 100) volumePercent = 100;

    // The clocks run before the codec is configured, as its driver expects.
    i2s.setPins(PIN_I2S_BCLK, PIN_I2S_WS, PIN_I2S_DOUT, PIN_I2S_DIN, PIN_I2S_MCLK);
    if (!i2s.begin(I2S_MODE_STD, kSampleRate, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
        Serial.println("Audio: I2S start failed");
        return;
    }
    if (!codecInit()) {
        Serial.println("Audio: ES8311 setup failed");
        i2s.end();
        return;
    }

    queue = xQueueCreate(16, sizeof(Note));
    if (!queue || xTaskCreatePinnedToCore(playerTask, "audio", 4096, nullptr, 2, nullptr, 1) != pdPASS) {
        Serial.println("Audio: player task failed");
        return;
    }
    codecOk = true;
    Serial.printf("Audio: ES8311 at 0x%02X, %lu Hz, volume %u\n", kCodecAddress, (unsigned long)kSampleRate,
                  unsigned(volumePercent));
}

bool present() { return codecOk; }

void tone(uint16_t hz, uint16_t ms) {
    if (!codecOk) return;
    if (hz != 0 && hz < 50) hz = 50;
    if (hz > 8000) hz = 8000;
    if (ms < 10) ms = 10;
    if (ms > 5000) ms = 5000;
    enqueue(hz, ms);
}

void chime() {
    if (!codecOk) return;
    enqueue(880, 120);
    enqueue(0, 30);
    enqueue(1320, 180);
}

void play(uint8_t sound) {
    if (!codecOk) return;
    switch (sound) {
        case 1:
            enqueue(1500, 150);
            break;
        case 2:
            chime();
            break;
        case 3:
            for (int i = 0; i < 3; i++) {
                enqueue(900, 110);
                enqueue(1300, 110);
            }
            break;
        default:
            break;
    }
}

void setVolume(uint8_t percent) {
    volumePercent = percent > 100 ? 100 : percent;
    volumeDirty = true;
}

void poll() {
    if (!volumeDirty) return;
    volumeDirty = false;
    Preferences prefs;
    if (prefs.begin(kPrefs, false)) {
        prefs.putUChar(kVolumeKey, volumePercent);
        prefs.end();
    }
    if (codecOk) applyVolume();
}

uint8_t volume() { return volumePercent; }

}  // namespace audio
