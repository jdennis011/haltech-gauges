#pragma once

#include <stdint.h>

#include <ArduinoJson.h>
#include <FS.h>

// The microSD card on the P4 hub (docs/sd-card.md): mounted at start when one
// is in the slot, used for the datalog. Other hub boards have no slot and
// report none. The card is FAT, so it reads in any PC.
//
// The card shares the SDMMC host with the board's Wi-Fi module, and a failed
// card init can take the host down with it, which restarts the hub. So an
// attempt is noted in flash first and the note cleared once the hub has run a
// while; a note still there at start means the last attempt ended badly, and
// the card is left alone until Mount is pressed on the page.
namespace sd_card {

// Mounts the card if there is one. Safe to call again later (a card put in
// after start): true when mounted. `force` tries even after a bad attempt.
bool begin(bool force = false);
// From the main loop: clears the attempt note once the hub has settled.
void poll(uint32_t nowMs);
// Before a deliberate restart (a firmware update): the note is cleared so the
// restart is not taken for a card that crashed the hub.
void forgetProbe();
bool mounted();
// Reads the card's free space afresh. Slow on a big card (it walks the
// allocation table under the filesystem's lock), so only the log's task calls
// it, at mount and now and then; everything else reads the figure it left.
void refreshSpace();
// The card's filesystem, for serving files; null without a card.
fs::FS* fs();
uint64_t totalBytes();
uint64_t freeBytes();
// {slot, mounted, type, size_mb, free_mb, error, skipped}
void statusJson(JsonObject out);

}  // namespace sd_card
