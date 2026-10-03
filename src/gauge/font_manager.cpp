#include "font_manager.h"

#include <vector>

extern "C" {
extern const uint8_t hg_font_sans_start[], hg_font_sans_end[];
extern const uint8_t hg_font_condensed_start[], hg_font_condensed_end[];
extern const uint8_t hg_font_carter_start[], hg_font_carter_end[];
extern const uint8_t hg_font_racing_start[], hg_font_racing_end[];
extern const uint8_t hg_font_trade_start[], hg_font_trade_end[];
extern const uint8_t hg_font_digital_start[], hg_font_digital_end[];
extern const uint8_t hg_font_mono_start[], hg_font_mono_end[];
extern const uint8_t hg_font_display_start[], hg_font_display_end[];
}

using hg::cfg::Font;

namespace {

struct Source {
    const uint8_t* start;
    const uint8_t* end;
};

Source sourceFor(Font family) {
    switch (family) {
        case Font::Condensed: return {hg_font_condensed_start, hg_font_condensed_end};
        case Font::Digital: return {hg_font_digital_start, hg_font_digital_end};
        case Font::Mono: return {hg_font_mono_start, hg_font_mono_end};
        case Font::Display: return {hg_font_display_start, hg_font_display_end};
        case Font::Carter: return {hg_font_carter_start, hg_font_carter_end};
        case Font::Racing: return {hg_font_racing_start, hg_font_racing_end};
        case Font::Trade: return {hg_font_trade_start, hg_font_trade_end};
        case Font::Sans: break;
    }
    return {hg_font_sans_start, hg_font_sans_end};
}

struct Entry {
    Font family;
    uint16_t size;
    lv_font_t* font;
};

std::vector<Entry> cache;

// Glyphs cached per font instance. A gauge face shows few distinct characters.
const size_t kGlyphCache = 48;

}  // namespace

const lv_font_t* fontGet(Font family, uint16_t size) {
    for (const Entry& e : cache) {
        if (e.family == family && e.size == size) return e.font;
    }

    const Source src = sourceFor(family);
    lv_font_t* font = lv_tiny_ttf_create_data_ex(src.start, size_t(src.end - src.start), size,
                                                 LV_FONT_KERNING_NORMAL, kGlyphCache);
    if (!font) return LV_FONT_DEFAULT;

    cache.push_back({family, size, font});
    if (family != Font::Sans) font->fallback = fontGet(Font::Sans, size);
    return font;
}

void fontReleaseAll() {
    for (const Entry& e : cache) lv_tiny_ttf_destroy(e.font);
    cache.clear();
}
