#include "sd_card.h"

#include <Arduino.h>
#include <Preferences.h>
#include <sdkconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>

#if CONFIG_IDF_TARGET_ESP32P4
#include <diskio_impl.h>
#include <diskio_sdmmc.h>
#include <driver/sdmmc_host.h>
#include <esp_vfs_fat.h>
#include <ff.h>
#include <sdmmc_cmd.h>
#include <vfs_api.h>

#include <memory>
#define HG_HAS_SD 1
#else
#define HG_HAS_SD 0
#endif

using hg::sd::FormatPlan;
using hg::sd::FsKind;

namespace sd_card {

namespace {

const char* kMount = "/sd";
const char* kPrefs = "hub";
const char* kProbeKey = "sd_probe";
const uint32_t kSettledMs = 40000;  // an attempt that did not restart the hub by then was harmless
const int kPowerPin = 45;           // the slot's supply, on when low (the board's variant says so)
const char* kLabel = "OAGAUGE";     // the volume label a formatted card gets

bool isMounted = false;
bool skipped = false;   // left alone at start because the last attempt ended in a restart
bool probing = false;   // an attempt is noted in flash and not yet cleared
char errorText[96] = "";

// What the page reads from other tasks while the log's task owns the card.
std::atomic<bool> cardUp{false};        // a card answered (with or without a filesystem)
std::atomic<uint32_t> cardSectors{0};
std::atomic<uint32_t> cardSectorBytes{512};
std::atomic<uint8_t> fsType{0};         // FatFs's FS_FAT12/16/32 when mounted
char cardType[12] = "";

void noteProbe(bool on) {
    Preferences p;
    p.begin(kPrefs, false);
    if (on) {
        p.putUChar(kProbeKey, 1);
    } else {
        p.remove(kProbeKey);
    }
    p.end();
}

bool probeNoted() {
    Preferences p;
    p.begin(kPrefs, true);
    const bool noted = p.getUChar(kProbeKey, 0) != 0;
    p.end();
    return noted;
}

void sizeText(uint64_t bytes, char* out, size_t n) {
    const double mb = double(bytes) / (1024.0 * 1024.0);
    if (mb >= 1024) {
        snprintf(out, n, "%.1f GB", mb / 1024.0);
    } else {
        snprintf(out, n, "%.0f MB", mb);
    }
}

#if HG_HAS_SD
// The card is brought up with the chip's SD driver directly rather than the
// core's SD_MMC library: that library mounts through a helper whose failure
// path deletes the slot's LDO, which also powers the Wi-Fi module's lines on
// this board, so a card that would not read took the Wi-Fi down with it. Here
// the LDO is left as the Wi-Fi driver set it up, the data lines get the
// chip's internal pull-ups, and a failure only releases the slot.
sdmmc_card_t* card = nullptr;
FATFS* fatfs = nullptr;
BYTE drive = 0xFF;
std::shared_ptr<VFSImpl> impl;
fs::FS* arduinoFs = nullptr;
int busWidth = 0;
int busKhz = 0;
uint64_t cachedTotal = 0;
uint64_t cachedFree = 0;

// Slower first: at 20 MHz this slot read fine but writes failed with CRC
// errors, and one data line would not come up at all. Each setting must
// write and read back 32 KB before it is trusted.
const struct {
    int width, khz;
} kTries[] = {{4, 10000}, {4, 5000}, {4, 20000}, {4, 400}, {1, 10000}, {1, 400}};

void powerCycle() {
    // The slot's supply: off, then on, so a card put in while running starts clean.
    pinMode(kPowerPin, OUTPUT);
    digitalWrite(kPowerPin, HIGH);
    delay(50);
    digitalWrite(kPowerPin, LOW);
    delay(100);
}

bool tryCard(int width, int khz) {
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_0;
    host.max_freq_khz = khz;
    host.flags = width == 4 ? SDMMC_HOST_FLAG_4BIT : SDMMC_HOST_FLAG_1BIT;
    // Pins at 0 mean the slot's own IO MUX pins, as the core's library does for this slot.
    sdmmc_slot_config_t slot = {};
    slot.cd = SDMMC_SLOT_NO_CD;
    slot.wp = SDMMC_SLOT_NO_WP;
    slot.width = uint8_t(width);
    slot.flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    esp_err_t e = sdmmc_host_init();
    if (e != ESP_OK && e != ESP_ERR_INVALID_STATE) {  // already up for the Wi-Fi module is fine
        snprintf(errorText, sizeof(errorText), "SD host: %s", esp_err_to_name(e));
        return false;
    }
    e = sdmmc_host_init_slot(SDMMC_HOST_SLOT_0, &slot);
    if (e != ESP_OK) {
        snprintf(errorText, sizeof(errorText), "SD slot: %s", esp_err_to_name(e));
        return false;
    }
    card = static_cast<sdmmc_card_t*>(calloc(1, sizeof(sdmmc_card_t)));
    e = sdmmc_card_init(&host, card);
    if (e != ESP_OK) {
        snprintf(errorText, sizeof(errorText), "card did not answer (%d-bit, %d kHz): %s", width, khz, esp_err_to_name(e));
        free(card);
        card = nullptr;
        sdmmc_host_deinit_slot(SDMMC_HOST_SLOT_0);  // the Wi-Fi module's slot keeps the host up
        return false;
    }
    busWidth = width;
    busKhz = khz;
    cardSectors = uint32_t(card->csd.capacity);
    cardSectorBytes = card->csd.sector_size > 0 ? uint32_t(card->csd.sector_size) : 512;
    snprintf(cardType, sizeof(cardType), "%s", card->is_mmc ? "MMC" : (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC");
    cardUp = true;
    return true;
}

// Undoes mountFat(): the filesystem and its drive letter go, the card stays up.
void releaseFs() {
    if (fatfs) {
        char drv[3] = {char('0' + drive), ':', 0};
        f_mount(nullptr, drv, 0);
        esp_vfs_fat_unregister_path(kMount);
        ff_diskio_unregister(drive);
        fatfs = nullptr;
    }
    delete arduinoFs;
    arduinoFs = nullptr;
    impl.reset();
    fsType = 0;
    isMounted = false;
}

bool mountFat() {
    if (ff_diskio_get_drive(&drive) != ESP_OK) {
        snprintf(errorText, sizeof(errorText), "no free drive letter");
        return false;
    }
    ff_diskio_register_sdmmc(drive, card);
    char drv[3] = {char('0' + drive), ':', 0};
    esp_err_t e = esp_vfs_fat_register(kMount, drv, 5, &fatfs);
    if (e != ESP_OK && e != ESP_ERR_INVALID_STATE) {
        snprintf(errorText, sizeof(errorText), "could not register the card: %s", esp_err_to_name(e));
        ff_diskio_unregister(drive);
        return false;
    }
    const FRESULT fr = f_mount(fatfs, drv, 1);
    if (fr != FR_OK) {
        snprintf(errorText, sizeof(errorText),
                 "no filesystem the hub can read on the card (FatFs %d): exFAT, or blank; Format puts FAT32 on it", int(fr));
        esp_vfs_fat_unregister_path(kMount);
        ff_diskio_unregister(drive);
        fatfs = nullptr;
        return false;
    }
    fsType = fatfs->fs_type;
    impl = std::make_shared<VFSImpl>();
    impl->mountpoint(kMount);
    arduinoFs = new fs::FS(impl);
    return true;
}

// Everything back to how it was before tryCard(), so the next setting can be tried.
void unmountAll() {
    releaseFs();
    if (card) {
        free(card);
        card = nullptr;
        sdmmc_host_deinit_slot(SDMMC_HOST_SLOT_0);
    }
    cardUp = false;
    cardType[0] = 0;
}

// A setting that reads is not one that writes, as this slot showed: a small
// file written and read back decides.
bool selfTest() {
    const char* name = "/sd/.hubtest";
    uint8_t out[1024];
    for (size_t i = 0; i < sizeof(out); i++) out[i] = uint8_t(i * 7 + 3);
    FILE* f = fopen(name, "wb");
    if (!f) return false;
    bool ok = true;
    for (int k = 0; k < 32 && ok; k++) ok = fwrite(out, 1, sizeof(out), f) == sizeof(out);
    ok = fclose(f) == 0 && ok;
    if (ok) {
        f = fopen(name, "rb");
        ok = f != nullptr;
        for (int k = 0; k < 32 && ok; k++) {
            uint8_t in[1024];
            ok = fread(in, 1, sizeof(in), f) == sizeof(in) && memcmp(in, out, sizeof(in)) == 0;
        }
        if (f) fclose(f);
    }
    remove(name);
    return ok;
}

uint64_t cardBytes() { return uint64_t(cardSectors.load()) * uint64_t(cardSectorBytes.load()); }

void fatSpace(uint64_t& total, uint64_t& freeBytes) {
    total = freeBytes = 0;
    if (!fatfs) return;
    char drv[3] = {char('0' + drive), ':', 0};
    DWORD freeClusters = 0;
    FATFS* fs = nullptr;
    if (f_getfree(drv, &freeClusters, &fs) != FR_OK || !fs) return;
    const uint64_t sector = cardSectorBytes.load();
    total = uint64_t(fs->n_fatent - 2) * fs->csize * sector;
    freeBytes = uint64_t(freeClusters) * fs->csize * sector;
}

const char* fsName(uint8_t t) {
    switch (t) {
        case FS_FAT12:
            return "FAT12";
        case FS_FAT16:
            return "FAT16";
        case FS_FAT32:
            return "FAT32";
        default:
            return "none";
    }
}

// Brings a card up on the first setting it answers at, with no filesystem
// involved: for a format. The caller has taken the slot down first.
bool rawCard() {
    powerCycle();
    for (const auto& t : kTries) {
        if (tryCard(t.width, t.khz)) return true;
        Serial.printf("SD card: %s\n", errorText);
    }
    return false;
}
#endif

}  // namespace

bool begin(bool force) {
#if HG_HAS_SD
    if (isMounted) return true;
    if (card) unmountAll();  // a card that answered but would not mount: from the top
    if (!force && probeNoted()) {
        skipped = true;
        snprintf(errorText, sizeof(errorText),
                 "the last attempt to read a card ended in a restart; take the card out, or press Mount to try again");
        Serial.println("SD card: left alone after the last attempt restarted the hub");
        return false;
    }
    noteProbe(true);
    probing = true;
    skipped = false;
    powerCycle();

    bool up = false;
    for (const auto& t : kTries) {
        if (!tryCard(t.width, t.khz)) {
            Serial.printf("SD card: %s\n", errorText);
            continue;
        }
        if (!mountFat()) {
            // The card answers but holds no filesystem: no other setting
            // helps. It stays up, so the page can say how big it is and
            // offer to format it.
            Serial.printf("SD card: %s\n", errorText);
            return false;
        }
        if (!selfTest()) {
            snprintf(errorText, sizeof(errorText), "writes fail at %d-bit %d kHz", t.width, t.khz);
            Serial.printf("SD card: %s\n", errorText);
            unmountAll();
            continue;
        }
        up = true;
        break;
    }
    if (!up) return false;
    isMounted = true;
    errorText[0] = 0;
    refreshSpace();
    Serial.printf("SD card: %llu MB %s, %llu MB free, %d-bit at %d kHz\n", (unsigned long long)(cardBytes() / (1024 * 1024)),
                  fsName(fsType), (unsigned long long)(cachedFree / (1024 * 1024)), busWidth, busKhz);
    return true;
#else
    (void)force;
    snprintf(errorText, sizeof(errorText), "this board has no card slot");
    return false;
#endif
}

void poll(uint32_t nowMs) {
    if (probing && nowMs >= kSettledMs) {
        probing = false;
        noteProbe(false);
    }
}

void forgetProbe() {
    probing = false;
    noteProbe(false);
}

bool mounted() { return isMounted; }

bool present() { return cardUp.load(); }

fs::FS* fs() {
#if HG_HAS_SD
    return isMounted ? arduinoFs : nullptr;
#else
    return nullptr;
#endif
}

bool plan(FsKind kind, FormatPlan& out) {
    if (!cardUp) return false;
    out = hg::sd::planFormat(kind, cardSectors.load(), cardSectorBytes.load());
    return true;
}

bool format(FsKind kind, char* report, size_t reportLen) {
#if HG_HAS_SD
    // The card, with its filesystem out of the way; or brought up afresh.
    if (card) {
        releaseFs();
    } else if (!rawCard()) {
        snprintf(report, reportLen, "no card answered: %s", errorText);
        return false;
    }
    // A format that ends in a restart is treated like any card attempt that did.
    noteProbe(true);
    probing = true;

    FormatPlan p;
    plan(kind, p);
    char cardSize[16], volSize[16];
    sizeText(p.cardBytes, cardSize, sizeof(cardSize));
    sizeText(p.volumeBytes, volSize, sizeof(volSize));
    if (p.tooSmall) {
        snprintf(report, reportLen, "the card (%s) is too small for %s", cardSize, hg::sd::name(kind));
        unmountAll();
        begin(true);
        return false;
    }

    // The drive letter and FatFs's view of the card, for the format only.
    if (ff_diskio_get_drive(&drive) != ESP_OK) {
        snprintf(report, reportLen, "no free drive letter");
        unmountAll();
        begin(true);
        return false;
    }
    ff_diskio_register_sdmmc(drive, card);
    char drv[3] = {char('0' + drive), ':', 0};
    // A bigger work area makes f_mkfs clear the tables in bigger writes.
    size_t workLen = 32768;
    void* work = malloc(workLen);
    if (!work) {
        workLen = FF_MAX_SS;
        work = malloc(workLen);
    }
    FRESULT fr = work ? FR_OK : FR_NOT_ENOUGH_CORE;
    const char* stage = "";
    if (fr == FR_OK) {
        // One partition of the planned size in a fresh partition table; what
        // the filesystem cannot address is left after it, unpartitioned.
        stage = "partitioning";
        const uint32_t sectorBytes = cardSectorBytes.load();
        LBA_t table[4] = {LBA_t(p.volumeBytes / sectorBytes), 0, 0, 0};
        fr = f_fdisk(drive, table, work);
        Serial.printf("SD card: partition of %lu sectors (%s) on a %s card: FatFs %d\n", (unsigned long)table[0], volSize, cardSize, int(fr));
    }
    if (fr == FR_OK) {
        // The volume is that partition, by number, for the format; FatFs
        // finds it by itself afterwards.
        stage = "making the filesystem";
        const PARTITION saved = VolToPart[drive];
        VolToPart[drive].pd = drive;
        VolToPart[drive].pt = 1;
        MKFS_PARM opt = {};
        opt.fmt = kind == FsKind::Fat32 ? FM_FAT32 : FM_FAT;
        opt.n_fat = 2;                                             // as every other tool makes them
        opt.align = (4u * 1024 * 1024) / cardSectorBytes.load();   // data area on a 4 MB boundary: the card's erase blocks
        opt.au_size = 0;                                           // FatFs picks the cluster size (32 KB on a big FAT32)
        fr = f_mkfs(drv, &opt, work, UINT(workLen));
        VolToPart[drive] = saved;
        Serial.printf("SD card: %s filesystem: FatFs %d\n", hg::sd::name(kind), int(fr));
    }
    free(work);
    ff_diskio_unregister(drive);

    // From the top: the slot down and up, the new filesystem mounted and
    // written to as any card is at start.
    unmountAll();
    const bool up = begin(true);
    if (fr != FR_OK) {
        snprintf(report, reportLen, "format failed while %s (FatFs %d)%s", stage, int(fr), up ? "" : "; the card is not mounted");
        return false;
    }
    if (!up) {
        snprintf(report, reportLen, "formatted %s, but the card would not mount afterwards: %s", hg::sd::name(kind), errorText);
        return false;
    }
    {
        char label[16];
        snprintf(label, sizeof(label), "%d:%s", int(drive), kLabel);
        f_setlabel(label);
    }
    if (p.capped) {
        snprintf(report, reportLen, "formatted: one %s partition of %s on the %s card; %s cannot address the rest, which is left unused",
                 fsName(fsType), volSize, cardSize, hg::sd::name(kind));
    } else {
        snprintf(report, reportLen, "formatted: one %s partition of %s, the whole %s card", fsName(fsType), volSize, cardSize);
    }
    return true;
#else
    (void)kind;
    snprintf(report, reportLen, "this board has no card slot");
    return false;
#endif
}

void refreshSpace() {
#if HG_HAS_SD
    if (!isMounted) return;
    uint64_t total, freeB;
    fatSpace(total, freeB);
    cachedTotal = total;
    cachedFree = freeB;
#endif
}

uint64_t totalBytes() {
#if HG_HAS_SD
    return isMounted ? cachedTotal : 0;
#else
    return 0;
#endif
}

uint64_t freeBytes() {
#if HG_HAS_SD
    return isMounted ? cachedFree : 0;
#else
    return 0;
#endif
}

void statusJson(JsonObject out) {
    out["slot"] = HG_HAS_SD != 0;
    out["present"] = cardUp.load();
    out["mounted"] = isMounted;
    out["skipped"] = skipped;
    out["error"] = errorText;
#if HG_HAS_SD
    if (cardUp) {
        out["type"] = cardType;
        out["size_mb"] = uint32_t(cardBytes() / (1024 * 1024));
        out["fs"] = fsName(fsType);
    }
    if (isMounted) {
        out["free_mb"] = uint32_t(cachedFree / (1024 * 1024));
        out["bus"] = String(busWidth) + "-bit " + String(busKhz / 1000) + " MHz";
    }
#endif
}

}  // namespace sd_card
