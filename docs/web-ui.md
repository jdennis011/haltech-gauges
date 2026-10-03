# Hub web UI

The hub runs a Wi-Fi access point and serves a single page from its firmware.
Nothing is installed on the phone or PC.

## Reaching it

1. Join the Wi-Fi network **HaltechGauges**, password **haltech123** (both set in
   `src/hub/hub_config.h`).
2. Open **http://192.168.4.1/**. Any address typed into the browser lands there too.

To use it from a PC on your home network instead, give the hub your network's
details once, either in the page's **Settings** tab or on the serial console
with `W<network>;<password>`. The hub then also joins that network, keeps the
access point running, and the serial console (`w`) and the Settings tab show the
address it was given.

## Tabs

| Tab | What it does |
|---|---|
| Gauges | One card per gauge, real or planned, with a live preview of the config it is assigned, drawn at the face it is on. Real gauges show online state, MAC, firmware, supply voltage, and buttons for identify, face change, brightness and reboot. The assignment dropdown is the "desired state": the hub pushes that config whenever the gauge's stored one differs. |
| Live data | Every Haltech channel with its current value, updated three times a second. Data comes from the ECU bus (C6 hub) or from the simulator. |
| Configs | The library of stored configs. Start from a template (`docs/templates.md` describes the fifteen built in) or import a file, edit the JSON with a live preview of the selected face, **Validate** (runs the gauge's own parser on the hub), **Save**, **Download**, and **Try on gauge**, which shows a config on a gauge without storing it. Widgets can be dragged on the preview to move them; a click selects one (dashed outline, highlighted in the panel below), arrow keys nudge it by a pixel or ten with shift, and positions snap to the face centre. **Add widget** puts a new dial, ring, bar, number, label, light or rim on the face with sensible defaults, selected and ready to drag; each row has a remove button. Under the preview, **Widget styles** lists the widgets on the selected face, each with pickers for its channel, unit and range, its text, colours (a theme role or a custom colour), rotation, text alignment, whether a number shows its unit, and its font and size, so changing what a face shows needs no JSON editing (one dropdown also sets every font in the config), and **Fonts on the hub** shows each bundled family drawn with the gauge's own font file. |
| Settings | Hub, access point and home-network details; memory and bus counters. |

### Planned gauges

**Add a planned gauge** creates a gauge on the hub before the hardware exists,
so configs can be assigned and faces laid out on the bench or with no gauge at
all. A planned gauge has a name, a synthetic MAC (`02:00:00:00:00:<id>`, an
address no real board can have), an assigned config and a chosen face, and its
card previews that face with live or simulated data. Up to eight are kept in
`/virtual.json` on the hub's filesystem, so they survive a restart.

**Link to hardware…** enters the MAC of the real gauge (shown on its serial
banner and on its own card once it has announced itself). The name and the
config assignment move to that MAC, and from then on the real gauge's card
carries the planned name. Until the hardware is seen, the card says "waiting
for" that MAC. **Rename** on a real gauge's card does the same thing in one
step. **Unlink** forgets the name but leaves the assignment in place.

The **simulator** chip at the top right makes the hub generate the full Haltech
broadcast with moving values, for testing without the car. With a gauge on the
bus it also sends the frames to the gauge.

## Development board

On the Lolin C3 Mini build (`hub_dev_c3`) everything above works except the ECU
bridge: the header chip says "no ECU bridge (dev board)" and the Live data tab
only has values while the simulator is on.

## REST API

All responses are JSON. Errors are `{"ok":false,"error":"..."}` with a 4xx status.

| Method and path | Purpose |
|---|---|
| GET `/api/status` | Hub state: uptime, memory, bus counters, simulator, Wi-Fi |
| GET `/api/gauges` | Roster, real gauges first then planned ones (`"virtual":true`, node `V<id>`) |
| POST `/api/gauges/{node}/identify` `{"seconds":5}` | Gauge shows its id |
| POST `/api/gauges/{node}/face` `{"index":0}` | Switch face |
| POST `/api/gauges/{node}/brightness` `{"level":200}` | 10-255 |
| POST `/api/gauges/{node}/reboot` | |
| POST `/api/gauges/{node}/try` (body: a config) | Show without storing |
| POST `/api/gauges/{node}/endtry` | Back to the stored config |
| GET `/api/channels/meta` | Every channel with its unit and the unit conversions |
| GET `/api/channels` | Current live values |
| GET `/api/configs` | Stored configs: name, size, crc, title, faces |
| GET / PUT / DELETE `/api/configs/{name}` | Read, store (validated) or delete one |
| POST `/api/validate` (body: a config) | Parser verdict without storing |
| GET `/api/assignments` | `{"AA:BB:CC:DD:EE:FF":"name"}` |
| PUT `/api/assignments/{mac}` `{"name":"x"}` | Assign; empty name clears |
| GET `/api/templates`, GET `/api/templates/{name}` | Built-in starter configs |
| GET `/api/fonts` | Bundled font families: `key`, `family`, `style` (from `assets/fonts/fonts.json`) |
| GET `/api/virtual` | Planned gauges: id, label, mac, face |
| POST `/api/virtual` `{"label":"x","mac":"…"}` | Add one; `mac` optional, else synthetic. Returns `{"ok":true,"id":n}` |
| PUT `/api/virtual/{id}` `{"label","face","mac"}` | Change any of the three; `"mac":""` goes back to synthetic |
| DELETE `/api/virtual/{id}` | Remove; clears a synthetic-MAC assignment |
| POST `/api/simulator` `{"enabled":true}` | |
| GET / PUT / DELETE `/api/wifi` | Home-network credentials `{"ssid","password"}` |

`/ws` is a WebSocket that pushes `{"t":"status"}` once a second,
`{"t":"gauges"}` on change, and `{"t":"live","v":{...}}` three times a second.
A push is skipped when a client has not drained the previous ones or free memory
is below 40 KB; `ws_skipped` in the status counts those.

`#live`, `#configs` and `#settings` on the page's address open that tab directly.

## Memory

The ESP32-C3 has about 140 KB free with Wi-Fi up, and a failed allocation
restarts the firmware (the status shows `"reset":"panic"` afterwards). The
server therefore streams the 21 KB channel list and stored configs rather than
buffering them, keeps the WebSocket queue short, answers 503 to large requests
when memory is short (browsers retry), refuses to parse a config when memory is
tight, and the page fetches the five fonts only when the editor opens. Under a
deliberately heavy test (a browser loading the page while six downloads run,
repeated four times with four WebSocket clients open) the lowest free heap was
34 KB. Two or three browsers on the page at once is fine; a dozen is not what
it is for.

## Working on the page

`web/index.html` is the whole UI. Open it straight from disk in a browser and
it runs against built-in mock data, so layout work needs no hub. The build
script gzips it into `src/hub/web_assets.h` (about 14 KB) along with the
templates in `assets/templates/` and the fonts in `assets/fonts/`.
