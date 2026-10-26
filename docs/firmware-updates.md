# Firmware updates over Wi-Fi

Once the hub is behind the dash, its USB port is out of reach and Wi-Fi is the only way
in. This is how a new hub build gets there, and how a bad one gets undone.

- A release is built on the PC and published on the site with one script.
- The hub, when it is on the home network, looks at the site once a day and says on its
  page when a newer build is there. It never installs one by itself.
- **Install** on the page downloads it into the hub's spare app slot, checks it, and
  restarts. If the new build does not run, the bootloader starts the old one again.
- Without the site, a `.bin` can be uploaded straight from the page.

Gauges get theirs from the hub, over the hub's own Wi-Fi: see "Gauges" below.

## Releasing a build

```
.\scripts\release_fw.ps1 -Notes "What changed, in a line"
```

1. Bump `HUB_FW_VERSION` in `src/hub/hub_config.h` (versions are `major.minor.patch`,
   compared as numbers: 0.2.10 is newer than 0.2.9).
2. Run the script. It builds `hub` (ESP32-C6) and `hub_p4` (ESP32-P4), copies each
   `firmware.bin` to `firmware/<chip>/`, writes a `manifest.json` beside it, and puts
   both into the site's KV store with wrangler (`npx wrangler login` first, once).
   `-Envs hub_p4` does one board; `-NoUpload` only builds.
3. The site serves them at once:

| Path | |
|---|---|
| `GET /fw/<chip>/manifest.json` | `{"board":"esp32p4","version":"0.3.1","size":1800680,"sha256":"…","date":"2026-10-16","notes":"…"}` |
| `GET /fw/<chip>/firmware.bin` | The app image |
| `GET /fw/manifests.json` | The manifests of every chip the site knows, for the Hubs card |

`<chip>` is `esp32c6` or `esp32p4`. The firmware directory is not in git; the site holds
the current release and nothing older. There is no upload route: the script writes KV
directly with the account's wrangler login.

## On the hub

**Checking.** 30 s after start, then every 24 h, when the hub is on the home network and
has 96 KB of memory free (TLS needs about 45 KB). A check reads the manifest for the
hub's chip and compares the version with its own. **Check now** on the page does it at
once. The check shares the cloud task with the account sync, so only one HTTPS session
runs at a time; it needs no account.

**Installing.** Only when asked, from the page or `POST /api/update/install`:

1. The hub opens the spare app slot (`Update.begin`) and streams the download into it in
   4 KB pieces, summing SHA-256 as it goes. The page shows the percentage.
2. The size must match the manifest, the SHA-256 must match, and the image header must
   be valid (`Update.end`). Any failure leaves the running build untouched and the reason
   on the page.
3. The hub notes in NVS what it is moving from and to, and restarts 1.5 s later.

**After the restart.** The bootloader starts the new build in "pending verify" state.
The hub marks it good after 10 s of running (`esp_ota_mark_app_valid_cancel_rollback`);
the core would otherwise do that immediately at start, and `verifyRollbackLater()` tells
it not to. If the new build crashes or hangs before then, the next start goes back to the
old build, which reports on its page that the update was undone. The 10 s is short on
purpose: a key-off soon after an update must not undo it.

A build flashed over USB with esptool is not an OTA image and is never rolled back.

**Uploading a file.** The Firmware card's **Upload a .bin** sends `.pio/build/<env>/
firmware.bin` (the app image, not the merged flash image) straight to the spare slot with
`PUT /api/update`. No SHA-256 is checked, but the image header is, and the rollback
covers the rest. Useful on the bench, or for a hub with no internet.

**Do it with the engine off.** Writing 2 MB of flash takes 10 to 30 s and the CAN
receive path may miss frames while it runs; the gauges are not harmed, but it is not a
thing to do on the road.

**Boards.** The C6 hub (`default_8MB.csv`) and the P4 hub (`large_littlefs_32MB.csv`)
have two app slots. The C3 development board (`huge_app.csv`) has one, so it reports "no
spare app slot" and takes firmware over USB only.

## On the page

Settings, **Firmware**: this build and board, the latest on the site, the state with a
progress percentage, what the last update did, **Check now**, **Install** (shown when a
newer build is there), **Upload a .bin**. The page is told by the status push, so it
follows a download that another browser started.

Settings on the site, **Hubs**: each hub's firmware, with the newer version in brackets
when the site has one for its chip.

## API

| Method and path | |
|---|---|
| GET `/api/update` | `{current, board, slot, state, progress, error, last, confirmed, checked_s, available: {version, size, date, notes} \| null}`. Also under `update` in `/api/status`. States: `idle`, `checking`, `up to date`, `available`, `downloading`, `installing`, `restarting`, `error` |
| POST `/api/update/check` | Check the site now |
| POST `/api/update/install` | Install the build the last check found. 409 if there is none or one is already running |
| PUT `/api/update` (body: the `.bin`, `Content-Type: application/octet-stream`) | Flash an uploaded image and restart. 400 with the reason if it is refused. The content type matters: a form type (curl's default for `--data-binary`) makes the web server collect the body as form fields until the heap runs out and the connection drops, so from the command line it is `curl -X PUT -H "Content-Type: application/octet-stream" --data-binary @firmware.bin http://<hub>/api/update` |

The hub's status report to the site (`POST /api/hubs/status`, every five minutes) carries
`chip`, so the Hubs card can say which build applies.

## Gauges

A gauge's first firmware goes in over its USB port (that write includes the bootloader
and partition table). After that:

1. The release script builds the gauge boards too (`gauge` for the Waveshare 1.75" gauge,
   `gauge_m5dial` for the M5Stack Dial), with the version from `src/gauge/version.h`,
   and publishes them on the site as `gauge-ws175` and `gauge-m5dial`. The steering
   wheel module (`wheel`, version from `src/wheel/version.h`) goes up as `wheel-c3` and
   is updated the same way: it reports board 3 (docs/steering-wheel.md).
2. On the hub's page, Settings, Firmware, **Gauge firmware**: **Check the site** reads
   the manifests; **Fetch** downloads a build onto the hub's filesystem (`/gfw/`), with
   the SHA-256 checked. The P4 hub has room for both; the C6 hub, with 1.5 MB in all,
   refuses a fetch that would leave less than 128 KB free. The Dial build is only worth
   fetching while a Dial is on the bench.
3. Each gauge reports its board in its `Info` message (`hwRevision`: 1 the Waveshare
   gauge, 2 the Dial) and its version. A card whose gauge is on an older version than
   the hub's build for its board shows **Update firmware**.
4. Pressing it sends that gauge a `WifiJoin` message (docs/images.md, "Over Wi-Fi") with
   a firmware entry: size, SHA-256 and version, plus the access point and a token good
   for five minutes. The gauge joins, streams `GET /gauge/firmware?token=…` into its
   spare app slot, checks the size and SHA-256, leaves the network and restarts.
5. The new build runs in "pending verify" state and is confirmed after ten seconds of
   healthy running, at the same point the gauge clears its restart counter. A build that
   crashes before then is rolled back by the bootloader, and the hub sees the old version
   in the gauge's next `Info`.

Do it with the engine off, one gauge at a time: a gauge on Wi-Fi is still drawing its
face, but its live data lags while it writes flash. The hub refuses a second transfer
while one is running.

Proven on 2026-10-26 with the first Waveshare gauge, linked to the P4 hub over Wi-Fi
rather than CAN: Check, Fetch (1.87 MB from the site in about 15 s), Update on the card;
the join frames landed, the gauge took the image, restarted and reported 0.1.2 twenty-one
seconds after the button, and stayed on it past the rollback window.

## What could still go wrong

- **Power cut mid-write.** The slot being written is not the one running; the next
  start is the old build. The hub reports an error and the download can be started
  again.
- **A build that runs but is wrong.** Rollback only catches a build that fails to start
  or crashes within 10 s. For anything else, release a fixed version, or upload the
  previous `firmware.bin` from the page.
- **A manifest for the wrong chip.** Refused by name: the hub only accepts a manifest
  whose `board` is its own.
- **No space on the site.** KV values are limited to 25 MB; a hub build is under 2 MB.
