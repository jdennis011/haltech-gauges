# Images on the gauges

Backgrounds, logos and picture icons for the faces. The images are JPEG or PNG files kept on
the hub, and configs use them by name. Each gauge gets the images its config uses over the
gauge bus (CAN), before the config itself, and only once. A later step adds Wi-Fi for large
transfers, with the gauge given the Wi-Fi details over CAN (see "Later: over Wi-Fi" below).

## Using one

1. **Configs** tab, **Images** card, **Add image**. Pick any picture the browser can open.
   - The page shrinks it to the size you choose: the whole face (466 px), 350, half the face
     (233), an icon (120 or 64), or its own size if that fits.
   - It then encodes it as JPEG, or as PNG if it has transparency. You can force either format,
     and set the JPEG quality.
   - Before you save, it shows the file size and roughly how long the file takes to reach each
     gauge.
2. Use it in a config, either:
   - an **image** widget (**Add widget**, type `image`), or
   - the face's background (the **Face background** row above the widget list).

```json
{ "schema": 1, "name": "Logo face", "faces": [ {
    "name": "Main", "bgImage": "carbon", "bgImageOpacity": 60,
    "widgets": [
      { "type": "image", "image": "logo", "y": 120, "w": 200, "h": 90, "fit": "contain" },
      { "type": "image", "image": "oil-can", "y": 380, "w": 64, "h": 64,
        "channel": "oil_pressure_light", "flash": true },
      { "type": "number", "channel": "rpm", "size": 96 } ] } ] }
```

- **Image widget:** drawn in a `w` x `h` box centred on `x`, `y`.
  - `fit`: `contain` shows the whole image in proportion; `cover` fills the box in proportion,
    cutting off the edges; `stretch` fills the box exactly.
  - It also takes `opacity` and `rotate`.
  - With a `channel`, the image shows only while its `on` condition holds: above 0.5 unless set,
    so while a status flag is on. It can flash. That turns a picture into a warning light.
- **Face background:** `bgImage` fills the round face behind the widgets. Below 100%,
  `bgImageOpacity` lets the face colour (`bg`) show through, which darkens a busy picture behind
  the numbers.

**Export all** includes the images, so a backup restores the faces whole. A single config's
**Download** does not include them: it names them, and they have to be on the hub it is
imported into.

## Limits

| | |
|---|---|
| Formats | **JPEG:** baseline only. The gauge decodes JPEG with LVGL's TJpgDec, which does not read progressive JPEG. **PNG:** any kind (lodepng). The page always produces baseline JPEG. A file uploaded another way is checked, and refused with a reason |
| Size | At most 466 x 466 pixels and 256 KB per file |
| On the hub | **Count and space:** up to 32 images, always leaving 64 KB of the filesystem free for configs and recordings. **Hub filesystems:** the C6 hub has 1.5 MB, shared with configs and recordings; the C3 development board has 896 KB |
| In a config | At most 16 different images |
| Online copy | **Signed in:** 32 images and 4 MB per account. **Signed out:** in the browser's own storage, which holds about 4 MB |

## How they reach the gauges

**Images are named by the CRC32 of their bytes.** On the way to a gauge, the hub adds an
`"images"` map to the config, for example `"images": {"logo": "1A2B3C4D", "carbon": "00FF00AA"}`.
The gauge stores each file as `/img/1A2B3C4D.jpg` and finds it through that map. Because the
CRC follows the content:

- the same picture is never sent to a gauge twice;
- a changed picture is a new name, so it is sent again;
- a config edit doesn't resend any images.

A config in the library never has the map: the hub refuses one that does, and the page strips it
before saving.

**For each gauge, before sending its config:**

1. For each image the config uses, the hub asks the gauge whether it has it. That is one frame
   each way: message type 14, `Asset`.
2. Images the gauge lacks go as a segmented transfer of kind `Image` (3). The hub reads them
   from its flash a block at a time, so a 256 KB image never has to fit in its RAM.
3. Then the config goes, as before.

**Edge cases:**

- **Gauge resets:** the hub forgets what a gauge has whenever the gauge says hello (after a reset
  or reflash), and asks again. That is one question per image; nothing is resent if the gauge
  still has it.
- **Unused images:** on storing a config, the gauge deletes the images that config no longer
  uses. The hub keeps track of that too.
- **Refused images:** a gauge may refuse an image (no room, or firmware without images). Three
  failed sends for a passing reason (a timeout, the gauge unplugged) count as a refusal too.
  - The hub then sends the config without it, and the gauge draws nothing in its place.
  - The hub tries again after the gauge next restarts.
- **Deleted images:** an image deleted from the hub is left out of the map, and the gauge draws
  nothing for it.
- **Try on gauge:** the images go first here too.

**What the gauge card shows:**

- `sending an image… 40%`
- `images 1 of 3`, while the gauge catches up
- `config in sync`
- `in sync, 1 image(s) refused`, when a gauge would not take one

### Time on the bus

| Condition | Speed |
|---|---|
| Simulated, bus to itself (1 Mbit/s, about 7 frames per millisecond; test `test_xfer_image_speed_on_a_clean_link`) | About 43 KB a second: 100 KB in 2.3 s |
| In the car, sharing the bus with live data forwarded to the gauges (which goes first) | Estimated at roughly 25 KB a second; the page uses this figure. Measure it on the bench |

- Images go to one gauge at a time, so three gauges using the same picture each receive it.
- Transfers happen only when something changed: first setup, a new or changed image, or a
  replaced gauge.
- A full-face JPEG at the page's default quality is usually 20 to 80 KB, so a few seconds per
  gauge.

### On the gauge

- **Storage:** images are stored on the gauge's LittleFS (7.9 MB on the Waveshare board) as
  `/img/<CRC>.jpg` or `.png`.
- **Writing:** an image arrives in PSRAM, and a background task writes it to flash in 4 KB
  pieces, so the CAN task, and the live data, never wait on flash.
- **M5Stack Dial:** it has no PSRAM, so images arrive in internal RAM; keep them small there.
- **Not done yet:** the gauge firmware does not draw faces yet (the LVGL renderer is still to
  come), and does not store configs yet. When it does:
  - storing a config calls `image_store::keepOnly()` with the CRCs from its map;
  - the renderer opens `image_store::path(crc)` through an LVGL file-system driver, with
    `LV_USE_TJPGD` and `LV_USE_LODEPNG` on and the image cache in PSRAM.

## Later: over Wi-Fi

CAN is fine for an occasional picture, but slow for big transfers. A gauge firmware image of 1 to
2 MB would take a minute or more per gauge, competing with live data. The plan:

1. **Hub access point:** the hub keeps its access point.
   - When it has something large for a gauge (a firmware update, or images above a size
     threshold), it sends that gauge the Wi-Fi details over CAN.
   - That is a segmented transfer of a new kind, `WifiJoin`, to that gauge alone. It carries the
     network name, the password, the hub's address and port, and a one-time token.
   - No new message type is needed (type 15 is the last free one).
2. **The gauge downloads it:**
   - It turns its Wi-Fi on, joins, and downloads `GET /gauge/image/<crc>?token=…` (the same
     CRC-named file).
   - It checks the CRC and stores the file, then answers the hub's `Asset` question with yes,
     and turns Wi-Fi off again.
   - If Wi-Fi fails within about 20 s, the hub falls back to CAN.
3. **Firmware updates:** these use the same path.
   - The gauge downloads `/gauge/firmware.bin` into its other 4 MB app slot, checks it and
     restarts.
   - The hub sees the new version in the gauge's `Info`.
4. **Security:**
   - The gauge bus is a wire inside the car, but anything tapped into the chain could read the
     credentials. So they go only to a gauge the hub knows, are kept in the gauge's RAM and never
     stored, and the download endpoints refuse requests without the token.
   - The access point allows 4 connections by default. Phones and three gauges could exceed
     that, so either raise `max_connection`, or have the gauges join one at a time and leave when
     done.

## API (the hub)

| Endpoint | |
|---|---|
| GET `/api/images` | `{images:[{name, size, crc, width, height, type}], free, total, max_bytes, max_count}` |
| GET `/api/images/{name}` | The file |
| PUT `/api/images/{name}` (the file as the body, `Content-Type: image/jpeg` or `image/png`) | Streamed to the filesystem, checked, stored; replaces one of the same name |
| DELETE `/api/images/{name}` | |

`GET /api/configs` lists the images each config uses (`images`). `GET /api/gauges` gives each
gauge:

- `desired_crc`: the CRC of the config as it goes to the gauge, with its map;
- `images: {need, have, refused}`;
- while a transfer runs, `push.kind` (`config`, `try` or `image`) with `acked` and `total`
  frames.

The serial console's `m` reads every stored image the way a transfer does and checks its CRC.
