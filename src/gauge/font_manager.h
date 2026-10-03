#pragma once

#include <lvgl.h>

#include "face_model.h"

// Returns a bundled font family at a pixel size (the em size, as in CSS),
// creating it on first use. Characters a family lacks are drawn from the sans
// family, which covers every unit symbol. Returns LVGL's default font if the
// font cannot be created.
const lv_font_t* fontGet(hg::cfg::Font family, uint16_t size);

// Frees every font created so far. Call only when no object still uses them,
// i.e. after the faces have been deleted.
void fontReleaseAll();
