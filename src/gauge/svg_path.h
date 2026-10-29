#pragma once

#include <lvgl.h>

// SVG path data (a `d` attribute) read into an LVGL vector path for ThorVG:
// the path widgets of a face and the boot splash's lettering.
namespace svg_path {

// Absolute and relative moves, lines, horizontal and vertical lines, cubic
// and quadratic curves with their shorthands, arcs and closes. Returns false,
// with the path as far as it got, on data it cannot read.
bool parse(const char* d, lv_vector_path_t* path);

}  // namespace svg_path
