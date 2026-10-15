#pragma once

#include <ArduinoJson.h>

// Keeps the hub's library in step with the owner's account on the online copy
// (oagauge.online), once the hub has been linked to it with a code from the
// site. Configs, images, theme colours and alerts flow both ways: a change on
// the site reaches the hub within a minute, a change on the hub's own page is
// sent up straight away. Runs on its own task, over HTTPS with the car's home
// network. docs/cloud-sync.md.
namespace cloud_sync {

enum class Kind : uint8_t { Config, Image, ThemeColours, Alerts };

// After wifi_ap::begin() and the stores. Starts the task; it idles until the
// hub is linked and on the home network.
void begin();

bool linked();
// Links with a code from the site (asynchronous: statusJson() shows the outcome).
void requestLink(const char* code);
void requestUnlink();
// Syncs now rather than at the next minute.
void requestSync();

// The hub's page changed or deleted something: send it up.
void noteLocalChange(Kind kind, const char* name);
void noteLocalDelete(Kind kind, const char* name);

// linked, email, hub id, name, state, last sync, last error, pending.
void statusJson(JsonObject out);

}  // namespace cloud_sync
