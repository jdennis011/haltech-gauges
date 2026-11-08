# SD card and the datalog

The P4 hub has a microSD slot. With a card in it the hub keeps a datalog of every drive:
every channel the ECU broadcasts, ten times a second, as CSV, a new file at each key-on.
Nothing to start or stop; a 32 GB card holds months of driving.

The recorder (`docs/web-ui.md`) is the other thing: a short, named recording on the hub's
own flash for playing back on the page. The datalog is for taking away and looking at in a
spreadsheet or a log viewer.

## The card

- Any microSD card. The hub reads FAT (FAT32, FAT16, FAT12) and nothing else: its FatFs is
  built without exFAT. Cards of 64 GB and up come formatted exFAT, and Windows will not put
  FAT32 on anything over 32 GB, so the page can format the card itself (below). A
  high-endurance card (the kind sold for dash cameras) is the right one: the log writes a
  little every second for the whole drive.
- The hub mounts it at start. A card put in later is mounted with **Mount** on the page.
  Taking it out while a log is being written loses the last second or so of that file.
- Only the P4 hub has a slot. The C6 and C3 hubs report "no slot" and log nothing.

## The log

- `/logs/2026-10-17_09-15-30.csv`, named from the hub's clock (`docs/time-of-day.md`); with
  no time set, `log-0001.csv` and counting.
- The first line names the columns: `time_ms`, then every channel, each with its unit in
  brackets, e.g. `coolant_temp (°C)`. Each row after that is the time since the file began
  and the current value of every channel that has data; a channel with none is left blank.
- Ten rows a second, written while live data is arriving (from the ECU, or the simulator on
  the bench), flushed to the card every second. A file closes five seconds after the data
  stops, or at 64 MB, when a new one starts. A key-off simply cuts the power; the last
  second may be missing.
- At about 100 channels a row is some 500 bytes: roughly 20 MB an hour.
- Logging stops when the card has less than 16 MB free; nothing is deleted on its own.
- `/logs/runs.csv` collects the drag timer runs the gauges report, a row per run
  (`docs/motion.md`), whatever the logging switch says.

**Logging** can be switched off on the page; the setting is kept in the hub's preferences.

## On the page

Settings, **SD card**: the card's size, filesystem and free space (or "no card" and
**Mount**), the logging switch, what is being written right now, and the logs on the card,
each with **Download** and **Delete**. A card that answered but holds nothing the hub can
read is shown with its size and the reason, and the format row below it.

## Formatting a card

**Format as** FAT32 or FAT16, then **Format card…**. The hub first says what the card will
get, then asks for the word `FORMAT` to be typed; anything else, or Cancel, does nothing.
Everything on the card goes, logs included. The format runs on the log's task (the page
polls until it is done, a few seconds to a minute on a big card), and the card comes back
mounted, written to and read back as at start, with the volume label `OAGAUGE`.

What the card gets is one partition, in a fresh partition table, of the most the chosen
filesystem can address; when that is less than the card, the hub says so before asking for
the confirmation, makes the partition that size, and leaves the rest of the card
unpartitioned:

| Filesystem | Largest volume the hub makes | Smallest | Cluster size |
|---|---|---|---|
| FAT32 | 2 TiB less a sector (32-bit sector numbers, in the partition table and in FatFs); every microSD sold fits, Windows's 32 GB limit does not apply | 64 MB | FatFs's choice: 32 KB from 2 GB up |
| FAT16 | 4.0 GB (65 525 clusters of 64 KB) | 8 MB | up to 64 KB |
| exFAT | not offered: the hub cannot read it | | |

The planner behind this is `lib/hg_sd/format_plan.h` (host tests in `test/test_native_sd`);
the card side is `sd_card::format()`: `f_fdisk` for the partition, `f_mkfs` on it with two
FATs and the data area on a 4 MB boundary (the card's erase blocks), then the usual mount
with its 32 KB write-and-read-back test. A format that ends in a restart is treated like
any card attempt that did: the next start leaves the card alone until Mount.

## API

| Method and path | |
|---|---|
| GET `/api/sd` | `{card: {slot, present, mounted, type, size_mb, fs, free_mb, bus, error}, datalog: {enabled, active, mounting, formatting, format_ok, format_result, paused, error, file, rows, bytes, seconds}, logs: [{name, bytes, writing}]}`; `present` is a card that answered, with or without a filesystem the hub can read (`fs` is `FAT12`/`FAT16`/`FAT32` or `none`); the card and datalog parts are also in `/api/status` as `sd` and `datalog` |
| POST `/api/sd/mount` | Mount a card put in after start |
| GET `/api/sd/format?fs=FAT32` | What formatting would make of the card: `{fs, card_mb, volume_mb, capped, too_small, formatting}`; `fs` is `FAT32` or `FAT16` (400 for anything else, exFAT included); 409 with no card |
| POST `/api/sd/format` `{"fs": "FAT32", "confirm": "FORMAT"}` | Erase and format the card as planned; `confirm` must be exactly `FORMAT` (400 otherwise); 409 while a format runs. Progress: `datalog.formatting`, then `datalog.format_ok` and `format_result` |
| POST `/api/sd/logging` `{"on": false}` | Logging on or off |
| GET `/api/sd/logs/{name}` | The file, as a download |
| DELETE `/api/sd/logs/{name}` | Delete it; refused while it is being written |

## On the bench (2026-10-17)

The Waveshare P4-WIFI6 module's own slot is marginal with the card tried (a 64 GB SDHC/
SDXC): the card answers every command and reads, and the hub's 32 KB write-and-read-back
test passes at 4-bit 10 MHz, but sustained writes then fail with CRC errors and timeouts
after a few KB, at every bus width and clock tried. Two logs of 536 KB and 160 KB did get
through, so it is signal integrity rather than a dead line. The hub now copes: the card is
mounted and written from its own task, a card that stops taking writes pauses logging
(the page says so; Mount tries again), the file listing and free space are the task's
last readings so the page never waits on a stalled card, and a crash during a card
attempt makes the next start skip the card. The Arduino `SD_MMC` library is not used on
this board: its failure cleanup deletes the regulator channel that also powers the Wi-Fi
module's lines, which took the hub down with it.

To try next: another card (a high-endurance 16 or 32 GB one), and a look at the module's
schematic for the slot's pull-ups; the carrier's own slot, if one is added, should have
10 k pull-ups on CMD and the data lines and short traces.

## Not built

Images and gauge firmware still live on the hub's flash; the card could hold them, and
longer recordings for playback, but nothing needs it yet.
