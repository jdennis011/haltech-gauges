# Images on the gauges

Backgrounds, logos and picture icons for the faces. The images are JPEG or PNG files kept on
the hub, and configs use them by name. Each gauge gets the images its config uses before the
config itself, and only once: small ones over the gauge bus (CAN), big ones over the hub's
own Wi-Fi, with the gauge given the Wi-Fi details over CAN (see "Over Wi-Fi" below).

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
- **Big images** go over Wi-Fi where the gauge can, the bus otherwise: see "Over Wi-Fi".

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
- **M5Stack Dial:** it has no PSRAM, so images arrive in internal RAM. It refuses files over
  40 KB (the hub then sends the config without them) and draws only images that decode to
  60 KB or less, about 170 x 170 pixels for a JPEG; a full-face photo is skipped with a
  note on its serial port.
- **Drawing:** the renderer (`src/gauge/face_renderer.cpp`) finds each image through the
  config's `images` map and `image_store::path(crc)`, and shows it as an LVGL image object read
  from the filesystem (`L:/img/<crc>.jpg`) and decoded by TJpgDec or lodepng, with the fit,
  rotation and opacity the config asks. Decoded images stay in LVGL's cache (2 MB of PSRAM on the
  Waveshare board) so a background is decoded once, not every frame. Storing a config calls
  `image_store::keepOnly()` with the CRCs from its map, so the images it no longer uses go.
- **Where an image draws:** a face background sits under every widget; an image widget sits
  above every vector widget on the face, whatever its place in the config's order (the vector
  widgets are painted by one object, images are objects of their own on top). Put a shape over an
  image on the page and the gauge draws the image over the shape.
- **Missing images:** an image the hub never sent, or that the gauge has dropped, is simply not
  drawn; the page's preview shows a dashed box in its place.

## Over Wi-Fi

CAN is fine for an occasional picture, but slow for big transfers: a 100 KB background
takes a few seconds per gauge beside the live data, and a 1.1 MB gauge firmware image
would take a minute or more. Transfers from 24 KB up go over the hub's own Wi-Fi when the
gauge can, with CAN as the fallback. Gauge firmware always goes this way
(docs/firmware-updates.md, "Gauges").

**How a transfer goes:**

1. The hub finds, through the usual `Asset` questions, which images a gauge lacks. If any
   is 24 KB or more and the gauge has not been offered Wi-Fi since it last said hello, the
   hub sends it one `WifiJoin` message (transfer kind 5): a small JSON segmented transfer
   with the access point's name and password, the hub's address and port, a one-time
   token, and the CRCs to fetch.
2. The gauge turns its Wi-Fi on, joins the access point, and downloads
   `GET /gauge/image/<CRC>?token=…` for each, into PSRAM. It checks the CRC of the bytes,
   hands the image to its store, and tells the hub it has it with an unasked `Asset` reply,
   so the hub carries on at once. Then it leaves the network and turns Wi-Fi off; it is
   never on otherwise.
3. The hub gives the job 20 s plus the bytes at 50 KB/s. Images it has not heard about by
   then are asked about again; still missing, they go over CAN. A gauge whose firmware does
   not understand `WifiJoin` refuses it, and the images go over CAN at once. Either way the
   bus is tried only after Wi-Fi, once per hello.
4. The config follows when every image is there, as before.

**Security:** the gauge bus is a wire inside the car, but anything tapped into the chain
could read the frames. So the credentials go only to a gauge the hub knows, in a transfer
addressed to it, are kept in the gauge's RAM and never stored, and the download routes
answer only with a live token: one per job, 32 random hex characters, good for the job's
window plus a minute, and only for the images or firmware it was issued for. The access
point allows eight connections.

**Status:** a gauge reports a Wi-Fi job in progress with a flag in its status; the card
shows "fetching over Wi-Fi". The hub's serial log lists each image as it is stored.

**The M5Stack Dial** has no PSRAM, so images arrive in internal RAM there too; keep them
small on that board.

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
