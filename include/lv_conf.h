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

// Images the hub sends live on LittleFS as L:/img/<crc>.jpg or .png and are
// decoded by TJpgDec (baseline JPEG) and lodepng. Decoded images are kept in
// LVGL's cache so a face does not decode its background every frame: 2 MB of
// it in PSRAM on the Waveshare board; the M5Stack Dial has no PSRAM, so it
// keeps one small image's worth and decodes the rest as needed.
#define LV_USE_FS_ARDUINO_ESP_LITTLEFS 1
#define LV_FS_ARDUINO_ESP_LITTLEFS_LETTER 'L'
#define LV_FS_ARDUINO_ESP_LITTLEFS_PATH ""
#define LV_USE_TJPGD 1
#define LV_USE_LODEPNG 1
#ifdef HG_BOARD_M5DIAL
#define LV_CACHE_DEF_SIZE (64 * 1024)
#else
#define LV_CACHE_DEF_SIZE (2 * 1024 * 1024)
#endif
#define LV_IMAGE_HEADER_CACHE_DEF_CNT 16

// Path widgets (SVG path data) are drawn by ThorVG through LVGL's vector API,
// which also needs the matrix helpers.
#define LV_USE_MATRIX 1
#define LV_USE_VECTOR_GRAPHIC 1
#define LV_USE_THORVG_INTERNAL 1

#endif  // LV_CONF_H
