#pragma once

#include <stdint.h>

#include "channel_store.h"
#include "proto.h"

// Owns the CAN controller. A dedicated task receives frames, decodes Haltech
// broadcast data into the channel store, and runs the gauge's side of the
// management protocol. Requests from the hub that affect the screen are left
// as flags for the UI loop to pick up.
namespace can_task {

void begin();

hg::ChannelStore& channels();
uint32_t framesReceived();
bool hubPresent();
uint32_t nodeId();

// Reported to the hub in the status message.
void setVbusMillivolts(uint16_t millivolts);

// Identify: show this gauge's id until millis() reaches the returned time (0 = not requested).
uint32_t identifyUntilMs();
// Returns a brightness (0..255) the hub asked for, or -1 if there is none waiting.
int takeBrightnessRequest();
// A face index the hub asked for, or -1; a display request (0 off, 1 on), or -1.
int takeFaceRequest();
int takeDisplayRequest();
// The key-on intro the hub asked for, or a replay from the page; false if none is waiting.
bool takeIntro(hg::proto::Intro& out);

// The hub's time of day as seconds since 1970 in its zone, kept running from
// its last beacon; false if no hub has ever said (docs/time-of-day.md).
bool localTime(uint32_t& seconds);

// From the pull-down menu: ask the hub to apply a setting to every gauge.
void requestAll(hg::proto::Cmd cmd, uint8_t arg);

// --- from the serial console, for a gauge on the bench with no hub
void benchFace(int index);
void benchBrightness(int level);
void benchIntro(const hg::proto::Intro& intro);
void benchLocalTime(uint32_t seconds);

// --- from the Wi-Fi task
// An image fetched over Wi-Fi is now stored: the hub is told from the CAN task.
void reportImage(uint32_t crc);
// Restart once the bus has settled (after a firmware install).
void requestReboot();
bool rebootRequested();
// Whether the faces are showing the demo sweep, for the status message.
void setDemo(bool demo);

// The alert message the hub wants shown over the face. False when there is
// none; otherwise fills `text` (at least 33 bytes) and `color` (0xRRGGBB).
bool alertMessage(char* text, uint32_t& color);

// The hub's 32 theme colour slots (0xRRGGBB), for resolving "themecolour1" to 32
// in a face. The last ones the hub sent are kept in flash for running without a hub.
void themeColours(uint32_t out[32]);

}  // namespace can_task
