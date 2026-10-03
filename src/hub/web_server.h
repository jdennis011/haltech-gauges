#pragma once

// HTTP + WebSocket server for the hub's web UI. REST under /api, live
// updates on /ws, the page itself embedded in the firmware.
namespace web_server {

void begin();
// Sends the periodic WebSocket updates. Call from loop().
void loop();

}  // namespace web_server
