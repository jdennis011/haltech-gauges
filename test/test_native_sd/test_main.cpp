// The card format planner: which filesystem fits which card.

#include <unity.h>

#include "format_plan.h"

using namespace hg::sd;

void setUp() {}
void tearDown() {}

namespace {

const uint64_t kGiB = 1024ull * 1024 * 1024;
uint64_t sectors(uint64_t bytes) { return bytes / 512; }

void test_fat32_spans_a_32_gb_card() {
    const FormatPlan p = planFormat(FsKind::Fat32, sectors(32 * kGiB), 512);
    TEST_ASSERT_FALSE(p.capped);
    TEST_ASSERT_FALSE(p.tooSmall);
    TEST_ASSERT_EQUAL_UINT64(32 * kGiB, p.cardBytes);
    TEST_ASSERT_EQUAL_UINT64(32 * kGiB - 63 * 512, p.volumeBytes);
}

void test_fat32_spans_a_512_gb_card_windows_refuses() {
    const FormatPlan p = planFormat(FsKind::Fat32, sectors(512 * kGiB), 512);
    TEST_ASSERT_FALSE(p.capped);
    TEST_ASSERT_EQUAL_UINT64(512 * kGiB - 63 * 512, p.volumeBytes);
}

void test_fat32_stops_at_32_bit_sectors() {
    // A card past 2 TiB (more than 2^32 sectors) gets a 2 TiB partition.
    const FormatPlan p = planFormat(FsKind::Fat32, 0x1'0000'0000ull + 1000, 512);
    TEST_ASSERT_TRUE(p.capped);
    TEST_ASSERT_EQUAL_UINT64((0xFFFFFFFFull - 63) * 512, p.volumeBytes);
    TEST_ASSERT_TRUE(p.volumeBytes < p.cardBytes);
}

void test_fat16_is_capped_at_4_gb_on_a_16_gb_card() {
    const FormatPlan p = planFormat(FsKind::Fat16, sectors(16 * kGiB), 512);
    TEST_ASSERT_TRUE(p.capped);
    TEST_ASSERT_FALSE(p.tooSmall);
    TEST_ASSERT_EQUAL_UINT64(65500ull * 128 * 512, p.volumeBytes);
    TEST_ASSERT_TRUE(p.volumeBytes <= 4 * kGiB);
    TEST_ASSERT_TRUE(p.volumeBytes > 3900ull * 1024 * 1024);
}

void test_fat16_spans_a_2_gb_card() {
    const FormatPlan p = planFormat(FsKind::Fat16, sectors(2 * kGiB), 512);
    TEST_ASSERT_FALSE(p.capped);
    TEST_ASSERT_EQUAL_UINT64(2 * kGiB - 63 * 512, p.volumeBytes);
}

void test_fat32_refuses_a_32_mb_card_fat16_takes_it() {
    const uint64_t card = sectors(32ull * 1024 * 1024);
    TEST_ASSERT_TRUE(planFormat(FsKind::Fat32, card, 512).tooSmall);
    const FormatPlan p = planFormat(FsKind::Fat16, card, 512);
    TEST_ASSERT_FALSE(p.tooSmall);
    TEST_ASSERT_FALSE(p.capped);
}

void test_a_card_with_no_sectors_is_too_small() {
    TEST_ASSERT_TRUE(planFormat(FsKind::Fat32, 0, 512).tooSmall);
    TEST_ASSERT_TRUE(planFormat(FsKind::Fat32, 63, 512).tooSmall);
}

void test_parse_names() {
    FsKind k;
    TEST_ASSERT_TRUE(parseFsKind("FAT32", k));
    TEST_ASSERT_EQUAL(int(FsKind::Fat32), int(k));
    TEST_ASSERT_TRUE(parseFsKind("fat16", k));
    TEST_ASSERT_EQUAL(int(FsKind::Fat16), int(k));
    TEST_ASSERT_TRUE(parseFsKind("FAT12", k));
    TEST_ASSERT_FALSE(parseFsKind("exFAT", k));
    TEST_ASSERT_FALSE(parseFsKind("NTFS", k));
    TEST_ASSERT_FALSE(parseFsKind("", k));
    TEST_ASSERT_FALSE(parseFsKind(nullptr, k));
    TEST_ASSERT_EQUAL_STRING("FAT32", name(FsKind::Fat32));
    TEST_ASSERT_EQUAL_STRING("FAT16", name(FsKind::Fat16));
}

}  // namespace

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_fat32_spans_a_32_gb_card);
    RUN_TEST(test_fat32_spans_a_512_gb_card_windows_refuses);
    RUN_TEST(test_fat32_stops_at_32_bit_sectors);
    RUN_TEST(test_fat16_is_capped_at_4_gb_on_a_16_gb_card);
    RUN_TEST(test_fat16_spans_a_2_gb_card);
    RUN_TEST(test_fat32_refuses_a_32_mb_card_fat16_takes_it);
    RUN_TEST(test_a_card_with_no_sectors_is_too_small);
    RUN_TEST(test_parse_names);
    return UNITY_END();
}
