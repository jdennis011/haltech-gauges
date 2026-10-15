#pragma once

#include <stdint.h>

#include "channel_store.h"

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
