# Plan: the gauge designer and the `.gauge` file

A visual editor for gauge faces that runs in a browser, and a file format for the
result that carries everything a gauge needs, pictures included. It replaces the JSON
editor and the Widget styles panel in the hub's page, which stay as the fallback.

What exists today and is reused: the config format (`docs/config-schema.md`, schema 1),
its parser and tests (`lib/hg_config`), the browser renderer in `web/index.html`
(`drawFace`, which already draws all seven widget types and the fifteen templates), the
hub's library, assignment and push machinery, and the font set. What does not exist
yet: the gauge-side renderer (LVGL), which is phase 4 of the main plan. The designer and
its format are built first, on the PC, so the renderer is written once, against the
final format.

## 1. The `.gauge` file

A `.gauge` file is UTF-8 JSON, `schema: 2`. It is a superset of a schema 1 config: every
template and every stored config today is a valid `.gauge` once `schema` says 2.

```json
{
  "schema": 2,
  "name": "Street",
  "description": "NFS look with warning icons",
  "author": "james", "version": 3, "created": "2026-10-04T10:12:00Z",
  "theme": "nfs",
  "orientation": 0,
  "assets": {
    "cel":    { "kind": "png", "role": "mask",  "w": 64, "h": 64, "bytes": 1810, "data": "iVBORw0KGgo..." },
    "cruise": { "kind": "png", "role": "mask",  "w": 64, "h": 64, "bytes": 1623, "data": "..." },
    "logo":   { "kind": "png", "role": "image", "w": 160, "h": 48, "bytes": 5210, "data": "..." }
  },
  "faces": [
    { "name": "Street", "background": "logo", "widgets": [
      { "type": "icon", "asset": "cel", "x": 233, "y": 380, "w": 40, "h": 40,
        "channel": "check_engine_light", "when": { "flag": true },
        "offColor": "#333333", "onColor": "warn", "flash": false },
      { "type": "icon", "asset": "cruise", "x": 300, "y": 380, "w": 40, "h": 40,
        "channel": "cruise_control", "when": { "flag": true },
        "offColor": "dim", "onColor": "#39FF88", "hideWhenOff": true },
      { "type": "image", "asset": "logo", "x": 233, "y": 120, "w": 120, "h": 36, "opacity": 60 }
    ] }
  ]
}
```

Rules:

- **Assets** are PNG, base64 in `data`, with `w`, `h` and `bytes` stated so a reader can
  check them without decoding. `role: "mask"` means a white-on-transparent picture that
  the gauge recolours (one file serves every state and colour). `role: "image"` is drawn
  as is. SVG and JPEG are accepted by the designer, which rasterises them to PNG at the
  size placed, so the gauge only ever decodes PNG.
- **Limits**, enforced by the parser: file 512 KB, 8 faces, 24 widgets per face, of
  which at most 6 are pictures (icons, images and the background together), 32 assets in
  the file, each at most 466 x 466. Six full-size pictures decode to about 2.6 MB, which
  the S3's 8 MB PSRAM holds for the face being shown; the C3 dev hub never decodes
  anything.
- **Conditions** on any widget that lights up (icon, light, rim, warn): `when` is one of
  `{ "flag": true }`, `{ "above": x }`, `{ "below": x }`, `{ "between": [a, b] }`,
  `{ "equals": n }` (for enumerations such as gear), with optional `for_ms` so a value
  must hold before it counts, and `flash` as `true` or `{ "on": 250, "off": 250 }`.
  Later: `all` / `any` lists of conditions.
- **Orientation**: `orientation` 0, 90, 180 or 270 at the top level, for a gauge mounted
  on its side; the gauge turns the whole display. `rotate` in degrees on any widget that
  is a picture or text (label, number, icon, image, bar), as labels have today.
- **Colours** anywhere are a theme role (`fg`, `accent`, ...) or `#RRGGBB`. The theme is
  the six roles, as today; the designer edits them with a picker and shows the whole face
  recolouring.
- **Two new widget types**: `icon` (an asset mask, recoloured by state, with a condition)
  and `image` (an asset drawn as is, with scale, rotation and opacity). A face may name an
  asset as its `background`.
- Everything in schema 1 stays. Unknown keys are still ignored, so a newer designer's
  file loads on an older gauge minus what it does not know.

`.gauge` is the extension for export and import; inside the hub the library keeps
storing files by name. The hub and gauge accept schema 1 and 2.

## 2. Where it runs

One static web app, built to a single HTML file, deployed as two copies of the same
build:

| Copy | Where | What it can do |
|---|---|---|
| Online | Cloudflare, as a Worker serving static assets, at a `workers.dev` address until a domain is chosen. Cloudflare's Workers Builds deploys it from the GitHub repo on every push to `main`, and the README links to it | Design, preview with the built-in simulator, import and export `.gauge` files, share them. No hub needed |
| On the hub | The same build embedded in the firmware, as the page is today, served from the hub's own address | All of the above, plus live data in the preview, save to the hub's library, assign to a gauge, try on a gauge |

The online copy cannot talk to a hub directly. It is served over HTTPS, and browsers
refuse to let a secure page fetch from, or open a WebSocket to, a plain-HTTP address
such as the hub's on the LAN. Moving a design from the online copy to a hub is Export,
then Import on the hub's page: two clicks. Connected work happens in the hub's copy,
which has no such limit. (An HTTPS hub would lift it, but a certificate a browser trusts
for a LAN address is more trouble than it is worth here.)

Repo and deployment: the designer lives in `web/designer/` with its own `package.json`;
`npm run build` bundles it with esbuild into `web/designer/dist/`; a `wrangler.jsonc` at
the repo root points Cloudflare at that directory, and `scripts/embed_web.py` embeds the
same built file for the hub, so the two copies cannot differ. Cloudflare's free plan
covers it, and a Worker is also where a shared gallery could live later (KV or R2
storage) without changing hosts.

The channel list the designer needs comes from a `channels.json` generated at build time
from `haltech_channels.def`, bundled into the app, so the online copy works without a
hub, and the hub's `/api/channels/meta` can serve that same pre-built file instead of
generating it.

Build: plain ES modules, no framework. Expect about 250 KB gzipped with the icon library,
which the flash holds easily and the chunked file serving handles.

## 3. The designer

**Canvas.** The 466 px round face at the centre, zoomable, with the preview drawn by the
existing renderer at the hub's live rate or from the built-in simulator. Widgets are
selected by clicking, moved by dragging, resized and rotated by handles, nudged by arrow
keys, and snapped to the centre, to a circle of a chosen radius, and to each other.
Dials and rings show handles for start and sweep. Multi-select, duplicate, align,
distribute.

**Layers panel.** The face's widgets in draw order, drag to reorder, show/hide and lock.
Faces are tabs above the canvas: add, duplicate, rename, reorder, delete, with swipe
order being the tab order.

**Property panel.** Driven by a machine-readable description of every widget type and
property (`assets/schema/widgets.json`: key, type, range, default, enum values, which
widget types it applies to). The same file drives the hub page's Widget styles panel, the
docs table, and a native test that feeds every boundary value to the parser, so the
three cannot drift. Controls: channel picker with search grouped by unit kind; unit;
range; colours with the theme roles offered first; fonts with live samples; the
type-specific options already in the schema; conditions as a small form (channel,
operator, value, hold time, flash, colours per state).

**Asset manager.** Drop in PNG, JPEG or SVG; crop, scale, choose mask or image; set the
placed size; see the bytes it will add. A built-in icon library: the car subset of
Pictogrammers' Material Design Icons (Apache 2.0): engine, cruise control, high and low
beam, fog, ABS, traction, battery, oil, coolant, fuel, turbo, tyre, parking brake, doors,
indicators, and more. Icons are stored as SVG in the app and rasterised to a PNG mask at
export.

**Theme editor.** The six roles with a picker, presets, and "set every widget to" for
fonts, as the page has now.

**Files.** New from template (the fifteen built in), import `.gauge` or a schema 1
config, export `.gauge`, undo and redo, autosave to the browser's local storage so a
closed tab loses nothing. Connected to a hub: open from the library, save, assign to a
gauge, try on a gauge, and a status chip for the hub.

**Validation.** The parser's rules are mirrored in the app, so problems show while
editing (a widget off the face, a text too long, an asset too big). The hub remains the
authority: Save runs the hub's parser and shows its message.

## 4. Hub changes

- Uploads up to 512 KB, streamed straight into a temporary file in LittleFS as the body
  arrives (today the body is buffered in RAM and limited to 32 KB), then validated by a
  filtered parse that skips the base64 `data` strings, so a large file costs a few KB of
  heap. Then renamed into the library.
- Pushing streams the file from flash in blocks rather than from a RAM copy. At the
  measured link rate (about 30 KB/s), a 300 KB file reaches a gauge in about ten seconds,
  shown as progress on the gauge's card.
- Library listing reports schema, asset count and size per config; the planned-gauge
  previews keep working because the page's renderer learns icons and images too.
- The C6's partition table grows the LittleFS area (the default leaves about 1.5 MB; a
  dozen illustrated faces want more). Larger flash areas and the gauge's 16 MB layout
  need no change.

## 5. Gauge changes

- Parser: schema 2 (assets, icon, image, conditions, orientation, rotate), with the
  limits above. Native tests for every new rule, plus a "big file" test with real
  base64.
- Receive into PSRAM, verify, validate, write to LittleFS, keep last-known-good; all as
  planned, with the larger size.
- Renderer (main plan phase 4) written against schema 2: LVGL image objects fed by the
  PNG decoder (`LV_USE_LODEPNG`) with the image cache in PSRAM; mask icons use LVGL's
  image recolour; conditions evaluated at the channel-store rate with hold times and
  flash timers; display rotation for `orientation`.
- Memory check on the gauge: decode every asset of a face when the face is shown, free
  when it is left; refuse a face whose decoded size exceeds the budget rather than
  crashing, and report that in STATUS so the hub's card can say so.

## 6. Phases

| # | Deliverable | Verification |
|---|---|---|
| D1 | Schema 2 spec in `docs/config-schema.md`, `assets/schema/widgets.json`, parser and tests, `channels.json` generator, five sample `.gauge` files with icons | Native tests pass; every template re-exported as schema 2 parses; the hub's `/api/validate` accepts the samples with the filtered parse |
| D2 | Hub: streamed upload, filtered validation, file-streamed push, bigger partition | A 400 KB file uploads with free heap never below 40 KB; pushes to a gauge stub over CAN and verifies by CRC |
| D3 | Designer MVP, standalone: canvas with select, move, resize, rotate; layers; property panel from the schema; theme; faces; import and export; undo; autosave; templates; Cloudflare deployment from GitHub | Build a face from each template without typing JSON; export, reload the page, import, identical file; a push to `main` appears at the `workers.dev` address within minutes |
| D4 | Assets and icons: asset manager, icon library, mask recolour in the preview, conditions form | Check-engine and cruise icons grey or lit from the simulator; file size shown matches the export |
| D5 | The hub's copy: embedded build in the firmware; open from and save to the library, assign, try on gauge, live data in the preview | The hub's page links to its designer; a face saved there appears on a planned gauge's card within seconds; a file exported online and imported on the hub is byte-identical |
| D6 | Gauge renderer for schema 2 (joins main plan phase 4) | Each sample `.gauge` photographed on the gauge and compared with the designer's preview; a 466 x 466 background plus six icons keeps the needle at 30 fps |
| D7 | Polish: snapping, alignment, keyboard shortcuts, help overlay, a gallery page of shared files | A new user designs a boost gauge with a warning icon in under ten minutes from the help alone |

D1 to D5 are PC-side and need no hardware beyond the C3 hub. D6 waits on the gauge
bring-up. Rough effort: D1 and D2 a few days each, D3 the largest at one to two weeks,
D4 and D5 a few days each, D7 ongoing.

## 7. Decisions made

- **Hosting.** A Cloudflare Worker serving the static build at a `workers.dev` address,
  no domain for now; source on GitHub, with Workers Builds deploying from it and the
  README linking to the worker. The hub carries the same build. Because the online copy
  is HTTPS and the hub is HTTP, the online copy is standalone (section 2).
- **Pictures as PNG**, base64 inside the JSON. PNG keeps files small and the designer
  simple; the S3 decodes a full-face PNG in well under a second and caches it. Base64
  costs a third more bytes on the wire for the pictures only.
- **Pictures per face: 6**, counting icons, images and the background. Widgets per face
  rises from 16 to 24 so the dense templates and six pictures fit together.
- **No framework.** Vanilla modules plus esbuild keep the page small and match the
  existing code; the schema-driven property panel removes most of the work a framework
  would have saved.

## 8. Not in this plan

Fonts uploaded in a `.gauge` file (the five bundled fonts stay the set; adding a font is
a firmware build), animations, custom needle images (a later `needleAsset` on the dial
would be a small addition once images work), and sharing to a public gallery server.
