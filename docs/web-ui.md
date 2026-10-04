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
| Gauges | One card per gauge, real or planned, with a live preview of the config it is assigned, drawn at the face it is on, with the message and face of any alert in force. Real gauges show online state, MAC, firmware, supply voltage, and buttons for identify, face change, brightness, screen off and on, and reboot. An **All gauges** strip sets brightness for every gauge (remembered by the hub and applied to any gauge that connects) and turns every screen off or on. The assignment dropdown is the "desired state": the hub pushes that config whenever the gauge's stored one differs. |
| Live data | Every Haltech channel with its current value, updated three times a second. Data comes from the ECU bus (C6 hub) or from the simulator. The **Simulator** card pauses the values, runs them slower or faster (0.1x to 4x), and holds any channel at a value or sweeps it between limits of your own, which is how a face is laid out against a particular reading. |
| Alerts | Rules the hub checks against the live data ten times a second: when a channel goes above or below a value, leaves or enters a range, a status flag comes on or goes off, or a reading changes, the gauges show a message, switch to a face, or both, and go back when it ends. Each row shows whether the alert is in force and the current reading, and **Test** fires it for five seconds. `docs/alerts.md` has the details. |
| Configs | The library of stored configs. Start from a template (`docs/templates.md` describes the fifteen built in) or import a file, **Export all** to save every stored config, the planned gauges, the assignments and the alerts as one file, and **Import all** to restore such a file on this or another hub (configs of the same name are replaced; planned gauges are matched by MAC, or by name when not linked to hardware), edit the JSON with a live preview of the selected face, **Validate** (runs the gauge's own parser on the hub), **Save**, **Download**, and **Try on gauge**, which shows a config on a gauge without storing it. Widgets can be dragged on the preview to move them; a click selects one (dashed outline, highlighted in the panel below), arrow keys nudge it by a pixel or ten with shift, and positions snap to the face centre. While a widget is dragged, guides appear and snap its centre to the face centre and to other widgets' centres, horizontally and vertically. The four corner handles resize: the radius of a dial, ring or rim, the width and height of a bar, the text size of a number or label, the size of a light, the width and height of a shape (a polygon's corners scale), the size of a path. The knob above the selection rotates a text, number, light or bar widget about its position, turns the start angle of a dial or ring, or swings an arched label round its circle, snapping to 15 degree steps. A label follows a circle when its Arc radius is set. **Add widget** puts a new dial, ring, bar, number, label, light, rim, shape or path (a vector icon from SVG path data) on the face with sensible defaults, selected and ready to drag; each row has a remove button. A shape is a rectangle, ellipse, line, triangle or polygon; a path draws any icon from its SVG path data, with a few to pick from (engine, warning, battery, thermometer, arrow, circle, star). Either is plain decoration, or, given a channel under All options, lit while its condition holds and dimmed or hidden otherwise, which makes a path a warning icon. The rows are the draw order, later rows on top; drag a row by its title, or use the up and down arrows on it, to reorder them. Under the preview, **Widget styles** lists the widgets on the selected face, each with pickers for its channel, unit and range, its text, colours (a theme role or a custom colour), rotation, text alignment, whether and where a number shows its unit, and its font and size, plus an **All options** section listing every other setting the format accepts for that widget, with dropdowns for the choices and small editors for zones and warning thresholds, so changing what a face shows needs no JSON editing (one dropdown also sets every font in the config), and **Fonts on the hub** shows each bundled family drawn with the gauge's own font file. |
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

### Recordings

The **Recordings** card on the Live data tab records the live channel values (from the
ECU, or from the simulator on the bench) ten times a second for 30 seconds to 3
minutes, keeping only the values that changed, so a minute of driving is a hundred or
two kilobytes on the hub's filesystem. Play a recording and the simulator carries it
to every gauge on the bus and to the previews, following the simulator's pause and
speed, looping if asked; the simulator's overrides still apply on top. Download a
recording to keep it, to open it in the online copy (Upload recording there, then
Play), or to upload it to another hub. The **Library** row lists base recordings
published with the site (`assets/recordings` in the repo): the online copy plays them
directly, and a hub's page copies one onto the hub and plays it. Signed in to the
online copy, your own recordings and the simulator's speed and overrides are kept in
your account. A recording is JSON, one row per line:

```
{"kind":"haltech-gauges-recording","schema":1,"name":"highway","rate_hz":10,"channels":["rpm",...],"rows":[
[0,[0,2850,1,101.3,...]],
[100,[0,2862]],
...
[59900,[]]
],"duration_ms":60000}
```

Each row is the time in milliseconds and index-value pairs for the channels that
changed, indexes into the header's channel list.

The **simulator** chip at the top right makes the hub generate the full Haltech
broadcast with moving values, for testing without the car. With a gauge on the
bus it also sends the frames to the gauge. Frames keep going out while it is paused,
so gauges hold the frozen values. Up to 16 channels can be overridden at once.

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
| POST `/api/gauges/{node}/display` `{"on":false}` | Screen off or on |
| POST `/api/gauges/all/brightness` `{"level":200}` | Every gauge; remembered and re-applied to newcomers |
| POST `/api/gauges/all/display` `{"on":false}` | Every screen off or on (not kept across a hub restart) |
| POST `/api/gauges/all/identify` | |
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
| GET `/api/recordings` | `[{name, bytes, duration_ms, recording}]` |
| GET / PUT / DELETE `/api/recordings/{name}` | Download, upload (the file as the body, up to 1 MB, streamed to the filesystem) or delete one |
| POST `/api/record` `{"action":"start","name":"x","seconds":60}` or `{"action":"stop"}` | Returns the recorder's state, also in `/api/status` as `recording` |
| POST `/api/playback` `{"action":"start","name":"x","loop":true}` or `{"action":"stop"}` | Returns the playback state, also in `/api/status` as `playback` |
| GET `/api/simulator` | `{enabled, paused, speed, overrides:[{channel, hold} or {channel, min, max}]}` |
| POST `/api/simulator` | Any of `enabled`, `paused`, `speed` (0.05-10), `clear`, and `overrides` as `{"rpm": {"hold": 3000}, "coolant_temp": {"min": 80, "max": 110}, "gear": null}`; values in the channel's stored unit. Returns the settings |
| GET `/api/alerts` | `{rules, states:[{active, count, value}], max}`; the rule format is in `docs/alerts.md` |
| PUT `/api/alerts` (body: the list of rules) | Replaces every alert; refused as a whole, naming the rule, if one is wrong |
| POST `/api/alerts/test` `{"index":0,"seconds":5}` | Puts one alert in force for a while |
| GET / PUT / DELETE `/api/wifi` | Home-network credentials `{"ssid","password"}` |

`/ws` is a WebSocket that pushes `{"t":"status"}` once a second,
`{"t":"gauges"}` on change, `{"t":"live","v":{...}}` three times a second, and
`{"t":"alerts","states":[...]}` when an alert fires or ends.
A push is skipped when a client has not drained the previous ones or free memory
is below 40 KB; `ws_skipped` in the status counts those.

`#live`, `#alerts`, `#configs` and `#settings` on the page's address open that tab directly.

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
