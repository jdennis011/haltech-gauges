// LVGL 9.4 configuration for the gauge. Only settings that differ from LVGL's
// defaults are listed; lv_conf_internal.h supplies the rest.

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16

// Use the C library allocator. With PSRAM enabled, allocations above 4 KB
// (draw layers, glyph caches) land in PSRAM instead of a fixed LVGL pool.
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

#define LV_DEF_REFR_PERIOD 16

// Fractional angles for arcs and needles.
#define LV_USE_FLOAT 1

#define LV_THEME_DEFAULT_DARK 1

#define LV_FONT_MONTSERRAT_20 1

// Bundled TTF families are rendered at any size by TinyTTF.
#define LV_USE_TINY_TTF 1
#define LV_TINY_TTF_FILE_SUPPORT 0
#define LV_TINY_TTF_CACHE_GLYPH_CNT 128
#define LV_TINY_TTF_CACHE_KERNING_CNT 256

#endif  // LV_CONF_H
