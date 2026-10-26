#include "sd_card.h"

#include <Arduino.h>
#include <Preferences.h>
#include <sdkconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

namespace sd_card {

namespace {

const char* kMount = "/sd";
const char* kPrefs = "hub";
const char* kProbeKey = "sd_probe";
const uint32_t kSettledMs = 40000;  // an attempt that did not restart the hub by then was harmless
const int kPowerPin = 45;           // the slot's supply, on when low (the board's variant says so)

bool isMounted = false;
bool skipped = false;   // left alone at start because the last attempt ended in a restart
bool probing = false;   // an attempt is noted in flash and not yet cleared
char errorText[96] = "";

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
    return true;
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
        snprintf(errorText, sizeof(errorText), "no filesystem the hub can read on the card (FatFs %d); format it FAT32 on a PC", int(fr));
        esp_vfs_fat_unregister_path(kMount);
        ff_diskio_unregister(drive);
        return false;
    }
    impl = std::make_shared<VFSImpl>();
    impl->mountpoint(kMount);
    arduinoFs = new fs::FS(impl);
    return true;
}

// Everything back to how it was before tryCard(), so the next setting can be tried.
void unmountAll() {
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
    if (card) {
        free(card);
        card = nullptr;
    }
    sdmmc_host_deinit_slot(SDMMC_HOST_SLOT_0);
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

uint64_t cardBytes() { return card ? uint64_t(card->csd.capacity) * uint64_t(card->csd.sector_size) : 0; }

void fatSpace(uint64_t& total, uint64_t& freeBytes) {
    total = freeBytes = 0;
    if (!fatfs) return;
    char drv[3] = {char('0' + drive), ':', 0};
    DWORD freeClusters = 0;
    FATFS* fs = nullptr;
    if (f_getfree(drv, &freeClusters, &fs) != FR_OK || !fs) return;
    const uint64_t sector = card ? card->csd.sector_size : 512;
    total = uint64_t(fs->n_fatent - 2) * fs->csize * sector;
    freeBytes = uint64_t(freeClusters) * fs->csize * sector;
}
#endif

}  // namespace

bool begin(bool force) {
#if HG_HAS_SD
    if (isMounted) return true;
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

    // The slot's supply: off, then on, so a card put in while running starts clean.
    pinMode(kPowerPin, OUTPUT);
    digitalWrite(kPowerPin, HIGH);
    delay(50);
    digitalWrite(kPowerPin, LOW);
    delay(100);

    // Slower first: at 20 MHz this slot read fine but writes failed with CRC
    // errors, and one data line would not come up at all. Each setting must
    // write and read back 32 KB before it is trusted.
    const struct {
        int width, khz;
    } tries[] = {{4, 10000}, {4, 5000}, {4, 20000}, {4, 400}, {1, 10000}, {1, 400}};
    bool up = false;
    for (const auto& t : tries) {
        if (!tryCard(t.width, t.khz)) {
            Serial.printf("SD card: %s\n", errorText);
            continue;
        }
        if (!mountFat()) {
            unmountAll();
            return false;  // the card answers but holds no filesystem: no other setting helps
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
    Serial.printf("SD card: %llu MB, %llu MB free, %d-bit at %d kHz\n", (unsigned long long)(cardBytes() / (1024 * 1024)),
                  (unsigned long long)(cachedFree / (1024 * 1024)), busWidth, busKhz);
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

fs::FS* fs() {
#if HG_HAS_SD
    return isMounted ? arduinoFs : nullptr;
#else
    return nullptr;
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
    out["mounted"] = isMounted;
    out["skipped"] = skipped;
    out["error"] = errorText;
#if HG_HAS_SD
    if (isMounted && card) {
        out["type"] = card->is_mmc ? "MMC" : (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC";
        out["size_mb"] = uint32_t(cardBytes() / (1024 * 1024));
        out["free_mb"] = uint32_t(cachedFree / (1024 * 1024));
        out["bus"] = String(busWidth) + "-bit " + String(busKhz / 1000) + " MHz";
    }
#endif
}

}  // namespace sd_card
