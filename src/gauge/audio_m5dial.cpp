#include "audio.h"

// The Dial's buzzer is not driven by this firmware.
namespace audio {

void begin() {}
bool present() { return false; }
void tone(uint16_t, uint16_t) {}
void chime() {}
void play(uint8_t) {}
void poll() {}
void setVolume(uint8_t) {}
uint8_t volume() { return 0; }

}  // namespace audio
