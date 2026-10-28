#pragma once

#include <stdint.h>

// The gauge's speaker (docs/gauge-audio.md). On the Waveshare board an ES8311
// codec on the shared I2C bus takes 16-bit audio over I2S and drives a small
// amplifier with a 2-pin speaker socket; the amplifier is switched on only
// while something plays. Tones are queued and rendered by a task of their own,
// so a caller never waits. The M5Stack Dial build has no audio.
namespace audio {

// After board::begin() (the I2C bus is up). Finds the codec, sets it up and
// starts the player; prints why if the board has no codec or it failed.
void begin();

// Whether a codec answered and the player is running.
bool present();

// Queue a sine tone: 50..8000 Hz for 10..5000 ms; 0 Hz is a rest.
void tone(uint16_t hz, uint16_t ms);

// The identify chime: two short rising notes.
void chime();

// One of the protocol's sounds (hg::proto::Sound as a number): 1 beep, 2 chime, 3 alarm.
void play(uint8_t sound);

// From loop(): writes a changed volume to the codec and to flash. The codec is
// written from the loop task, where the bus's other users run.
void poll();

// Speaker volume 0..100, kept in flash. 0 mutes the codec. Takes effect on the next poll().
void setVolume(uint8_t percent);
uint8_t volume();

}  // namespace audio
