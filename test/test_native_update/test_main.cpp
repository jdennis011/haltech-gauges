// Firmware update manifests: versions and the manifest the site serves.

#include <string.h>
#include <unity.h>

#include <string>

#include "update_manifest.h"

using namespace hg::update;

void setUp() {}
void tearDown() {}

namespace {

bool parse(const std::string& json, const char* board, Manifest& out, std::string& error) {
    return parseManifest(reinterpret_cast<const uint8_t*>(json.data()), json.size(), board, out, error);
}

void assertRefused(const std::string& json, const char* board, const char* expectInError) {
    Manifest m;
    std::string error;
    TEST_ASSERT_FALSE_MESSAGE(parse(json, board, m, error), json.c_str());
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(error.c_str(), expectInError), error.c_str());
}

const char* kGood =
    R"({"board":"esp32p4","version":"0.3.1","size":1800680,)"
    R"("sha256":"9F86D081884C7D659A2FEAA0C55AD015A3BF4F1B2B0B822CD15D6C15B0F00A08","date":"2026-10-16","notes":"Alerts switch configs"})";

}  // namespace

void test_versions_parse_and_compare() {
    Version v;
    TEST_ASSERT_TRUE(parseVersion("1.2.3", v));
    TEST_ASSERT_EQUAL(1, v.major);
    TEST_ASSERT_EQUAL(2, v.minor);
    TEST_ASSERT_EQUAL(3, v.patch);
    TEST_ASSERT_TRUE(parseVersion("v0.10", v));
    TEST_ASSERT_EQUAL(0, v.major);
    TEST_ASSERT_EQUAL(10, v.minor);
    TEST_ASSERT_EQUAL(0, v.patch);

    TEST_ASSERT_FALSE(parseVersion("", v));
    TEST_ASSERT_FALSE(parseVersion("1", v));
    TEST_ASSERT_FALSE(parseVersion("1.2.3.4", v));
    TEST_ASSERT_FALSE(parseVersion("1.2.x", v));
    TEST_ASSERT_FALSE(parseVersion("1..2", v));
    TEST_ASSERT_FALSE(parseVersion(" 1.2", v));
    TEST_ASSERT_FALSE(parseVersion(nullptr, v));

    TEST_ASSERT_TRUE(newerThan("0.3.0", "0.2.0"));
    TEST_ASSERT_TRUE(newerThan("0.2.10", "0.2.9"));  // numbers, not strings
    TEST_ASSERT_TRUE(newerThan("1.0.0", "0.99.99"));
    TEST_ASSERT_FALSE(newerThan("0.2.0", "0.2.0"));
    TEST_ASSERT_FALSE(newerThan("0.1.9", "0.2.0"));
    TEST_ASSERT_FALSE(newerThan("garbage", "0.2.0"));
    TEST_ASSERT_FALSE(newerThan("0.3.0", "garbage"));
}

void test_manifest_parses_for_its_board() {
    Manifest m;
    std::string error;
    TEST_ASSERT_TRUE_MESSAGE(parse(kGood, "esp32p4", m, error), error.c_str());
    TEST_ASSERT_EQUAL_STRING("0.3.1", m.version.c_str());
    TEST_ASSERT_EQUAL_STRING("esp32p4", m.board.c_str());
    TEST_ASSERT_EQUAL_UINT32(1800680, m.size);
    // The SHA-256 is kept lower case whatever the file has.
    TEST_ASSERT_EQUAL_STRING("9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08", m.sha256.c_str());
    TEST_ASSERT_EQUAL_STRING("2026-10-16", m.date.c_str());
    TEST_ASSERT_EQUAL_STRING("Alerts switch configs", m.notes.c_str());

    // Date and notes are optional.
    TEST_ASSERT_TRUE(parse(R"({"board":"esp32c6","version":"1.0.0","size":10,"sha256":")" + std::string(64, 'a') + "\"}", "esp32c6", m, error));
    TEST_ASSERT_TRUE(m.date.empty());
}

void test_bad_manifests_are_refused_with_a_reason() {
    assertRefused(kGood, "esp32c6", "for 'esp32p4', this hub is 'esp32c6'");
    assertRefused("not json", "esp32p4", "not JSON");
    assertRefused("[1,2]", "esp32p4", "not JSON");
    assertRefused(R"({"board":"esp32p4","version":"next","size":10,"sha256":"00"})", "esp32p4", "'version' is not a version");
    assertRefused(R"({"board":"esp32p4","version":"0.3.1","sha256":"00"})", "esp32p4", "'size' is missing");
    assertRefused(R"({"board":"esp32p4","version":"0.3.1","size":"big","sha256":"00"})", "esp32p4", "'size' is missing");
    assertRefused(R"({"board":"esp32p4","version":"0.3.1","size":10,"sha256":"00ff"})", "esp32p4", "'sha256' is not 64 hex");
    assertRefused(R"({"board":"esp32p4","version":"0.3.1","size":10})", "esp32p4", "'sha256' is not 64 hex");
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_versions_parse_and_compare);
    RUN_TEST(test_manifest_parses_for_its_board);
    RUN_TEST(test_bad_manifests_are_refused_with_a_reason);
    return UNITY_END();
}
