#pragma once

#include <stdint.h>

// Planning a card format on the hub (docs/sd-card.md, "Formatting a card").
//
// The hub's FatFs is built without exFAT and with 32-bit sector numbers, so
// the filesystems it can make are FAT32 and FAT12/16, and each can address
// only so much of a card. The plan says what the card would get: one
// partition of the largest size the filesystem allows, the rest unused, and
// whether that is the whole card. The sizes here follow FatFs's f_mkfs with
// its automatic cluster size, with a margin.
namespace hg {
namespace sd {

enum class FsKind : uint8_t { Fat32, Fat16 };

struct FormatPlan {
    FsKind kind = FsKind::Fat32;
    uint64_t cardBytes = 0;
    uint64_t volumeBytes = 0;  // the one partition the card gets
    bool capped = false;       // the filesystem cannot span the card
    bool tooSmall = false;     // the card is below the filesystem's minimum
};

inline const char* name(FsKind k) { return k == FsKind::Fat32 ? "FAT32" : "FAT16"; }

// "FAT32" or "FAT16" (also "FAT12"): false for anything else, exFAT included.
inline bool parseFsKind(const char* text, FsKind& out) {
    if (!text) return false;
    auto eq = [](const char* a, const char* b) {
        for (; *a && *b; a++, b++) {
            const char ca = (*a >= 'a' && *a <= 'z') ? char(*a - 32) : *a;
            if (ca != *b) return false;
        }
        return *a == 0 && *b == 0;
    };
    if (eq(text, "FAT32")) {
        out = FsKind::Fat32;
        return true;
    }
    if (eq(text, "FAT16") || eq(text, "FAT12") || eq(text, "FAT")) {
        out = FsKind::Fat16;
        return true;
    }
    return false;
}

// The first partition starts after the master boot record's track.
constexpr uint32_t kPartitionStartSectors = 63;

// The most sectors one volume of the kind can hold.
inline uint64_t maxVolumeSectors(FsKind k) {
    switch (k) {
        case FsKind::Fat32:
            // 32-bit sector numbers: the partition table and FatFs's LBA both
            // stop at 2^32 sectors; the cluster count (0x0FFFFFF5 of up to 64
            // sectors) is not the limit before that.
            return 0xFFFFFFFFull - kPartitionStartSectors;
        case FsKind::Fat16:
            // 65525 clusters of at most 128 sectors (64 KB); a few clusters
            // of margin for the FATs and root directory.
            return 65500ull * 128;
    }
    return 0;
}

// The fewest sectors f_mkfs makes a volume of the kind from.
inline uint64_t minVolumeSectors(FsKind k) {
    switch (k) {
        case FsKind::Fat32:
            return 64ull * 1024 * 1024 / 512;  // 65526 clusters of one sector, plus the tables
        case FsKind::Fat16:
            return 8ull * 1024 * 1024 / 512;  // more than 4085 clusters of one sector
    }
    return 0;
}

inline FormatPlan planFormat(FsKind kind, uint64_t cardSectors, uint32_t sectorBytes) {
    FormatPlan p;
    p.kind = kind;
    if (sectorBytes == 0) sectorBytes = 512;
    // The limits above are in 512-byte sectors; scale for a card that reports another size.
    const uint64_t scale = sectorBytes / 512 ? sectorBytes / 512 : 1;
    p.cardBytes = cardSectors * sectorBytes;
    if (cardSectors <= kPartitionStartSectors) {
        p.tooSmall = true;
        return p;
    }
    const uint64_t usable = cardSectors - kPartitionStartSectors;
    const uint64_t most = maxVolumeSectors(kind) / scale;
    const uint64_t least = minVolumeSectors(kind) / scale;
    if (usable < least) {
        p.tooSmall = true;
        return p;
    }
    p.capped = usable > most;
    p.volumeBytes = (p.capped ? most : usable) * sectorBytes;
    return p;
}

}  // namespace sd
}  // namespace hg
