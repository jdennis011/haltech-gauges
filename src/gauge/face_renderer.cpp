#include "face_renderer.h"

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "can_task.h"
#include "clock_text.h"
#include "channel_store.h"
#include "display.h"
#include "font_manager.h"
#include "image_store.h"
#include "units.h"

using namespace hg;
using namespace hg::cfg;

namespace face_renderer {

namespace {

constexpr float kPi = 3.14159265358979f;

// One widget being drawn, with what was drawn last so changes can be found.
struct Item {
    const Widget* w = nullptr;
    lv_area_t area = {};        // where it was last drawn, panel pixels
    bool valid = false;         // the value below is usable
    float value = 0;            // in display units
    uint32_t key = 0;           // the look last drawn, quantised
    bool holdSet = false;
    float hold = 0;
    std::string text;           // number: the text drawn
    lv_obj_t* label = nullptr;  // rotated text, as an LVGL label
    std::vector<lv_obj_t*> chars;  // arched text, a label per character
    lv_obj_t* image = nullptr;  // an image widget
    lv_vector_path_t* vpath = nullptr;  // a path widget's path
};

lv_obj_t* painter = nullptr;
lv_obj_t* screen = nullptr;
lv_obj_t* background = nullptr;  // the face's background image
std::vector<Item> items;
const Config* config = nullptr;
const Face* face = nullptr;
Theme theme;
float k = 1;  // panel pixels per design pixel
uint32_t now = 0;
bool everLive = false;
// The key-on intro (setIntro()); introSlot counts the widgets through update().
bool introOn = false;
hg::proto::IntroStyle introStyle = hg::proto::IntroStyle::Sweep;
float introPhase = 0;
size_t introSlot = 0;
size_t introSlots = 1;
bool anyLive = false;
uint32_t themeCols[kMaxThemeColours];
uint32_t lastThemeCheckMs = 0;

// ------------------------------------------------------------ helpers

int32_t S(float designPx) { return int32_t(lroundf(designPx * k)); }
float Sf(float designPx) { return designPx * k; }
float rad(float deg) { return deg * kPi / 180.0f; }

lv_color_t col(Color c) { return lv_color_hex(resolveColor(c, themeCols) & 0xFFFFFF); }

const lv_font_t* font(Font family, float designPx) {
    int px = S(designPx);
    if (px < 8) px = 8;
    return fontGet(family, uint16_t(px));
}

uint16_t fontPx(float designPx) {
    const int px = S(designPx);
    return uint16_t(px < 8 ? 8 : px);
}

float widgetX(const Widget& w) { return float(w.x); }
float widgetY(const Widget& w) { return float(w.y); }

bool flashOn(uint32_t periodMs) { return (now / periodMs) % 2 == 0; }

// The channel's value in the widget's display unit. Live when the bus has
// it; a slow sweep while no live data has ever arrived, so the layout can be
// judged; nothing once live data has stopped.
// A made-up reading for a widget: `p` from 0 to 1 across its range, lights
// on or off. The demo sweep and the key-on intro both use it.
void syntheticValue(const Widget& w, float p, bool lit, float& out) {
    switch (w.type) {
        case WidgetType::Number:
            out = w.format == NumberFormat::Gear ? floorf(p * 6.99f) : 100 * p;
            return;
        case WidgetType::Light:
        case WidgetType::Shape:
        case WidgetType::Path:
        case WidgetType::Image:
            out = lit ? 1.0f : 0.0f;
            return;
        case WidgetType::Rim: {
            const float lo = w.from * 0.8f, hi = w.rimMode == RimMode::Fill ? w.to : w.from * 1.2f;
            out = lo + (hi - lo) * p;
            return;
        }
        default:
            out = w.min + (w.max - w.min) * p;
            return;
    }
}

// The intro's sweep: up to full scale and back over the phase. With the
// chase style each widget starts a little after the one before it.
float introSweep() {
    float local = introPhase;
    if (introStyle == hg::proto::IntroStyle::Chase && introSlots > 1) {
        const float offset = 0.5f * float(introSlot) / float(introSlots);
        local = (introPhase - offset) / 0.5f;
        if (local < 0) local = 0;
        if (local > 1) local = 1;
    }
    return local < 0.5f ? local * 2 : (1 - local) * 2;
}

bool readValue(const Widget& w, float& out) {
    if (w.channel < 0) return false;
    float raw;
    const bool live = can_task::channels().get(ChannelId(w.channel), now, raw);
    if (live) {
        everLive = true;
        anyLive = true;
    }
    if (introOn) {
        const float s = introSweep();
        syntheticValue(w, s, s > 0.3f, out);
        return true;
    }
    if (live) {
        out = w.unit ? toDisplay(*w.unit, raw) : raw;
        return true;
    }
    if (everLive) return false;
    const float p = 0.5f - 0.5f * cosf(float(now % 4000) * (2.0f * kPi / 4000.0f));
    syntheticValue(w, p, (now / 3000) % 2 == 1, out);
    return true;
}

float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

// A widget's colour past its warning threshold.
Color applyWarn(const Widget& w, bool valid, float v, Color base) {
    const Warn& wn = w.warn;
    if (!wn.enabled || !valid) return base;
    const bool hit = wn.above ? v > wn.threshold : v < wn.threshold;
    if (!hit) return base;
    if (wn.flash && !flashOn(250)) return base;
    return wn.color;
}

Color zoneColor(const Widget& w, float v, Color base) {
    Color c = base;
    for (const Zone& z : w.zones) {
        if (v >= z.from && v <= z.to) c = z.color;
    }
    return c;
}

// Whether a light, shape, path or image with a channel is lit right now.
bool litNow(const Widget& w, bool valid, float v) {
    if (!w.warn.enabled) return true;
    const bool lit = valid && (w.warn.above ? v > w.warn.threshold : v < w.warn.threshold);
    return lit && (!w.warn.flash || flashOn(250));
}

// The text a number widget shows: plain, zero-padded, or a gear letter.
std::string numberText(const Widget& w, bool valid, float v) {
    char buf[32];
    if (w.format == NumberFormat::Gear) {
        if (!valid) return "-";
        const int g = int(lroundf(v));
        if (g == 0) return "N";
        if (g == -1) return "R";
        if (g == -2) return "P";
        if (g < 0) return "-";
        snprintf(buf, sizeof(buf), "%d", g);
        return buf;
    }
    if (!valid) return "---";
    snprintf(buf, sizeof(buf), "%.*f", int(w.decimals), double(v));
    if (!w.pad) return buf;
    std::string txt(buf);
    const bool neg = !txt.empty() && txt[0] == '-';
    if (neg) txt.erase(0, 1);
    const size_t dot = txt.find('.');
    std::string whole = dot == std::string::npos ? txt : txt.substr(0, dot);
    const std::string rest = dot == std::string::npos ? "" : txt.substr(dot);
    while (whole.size() < w.pad) whole.insert(0, "0");
    return (neg ? "-" : "") + whole + rest;
}

// A dial's tick label: whole numbers as such, otherwise one decimal.
std::string tickText(float val) {
    char buf[24];
    if (fabsf(val - roundf(val)) < 1e-4f) {
        snprintf(buf, sizeof(buf), "%d", int(lroundf(val)));
    } else {
        snprintf(buf, sizeof(buf), "%.1f", double(val));
    }
    return buf;
}

// ------------------------------------------------------- draw primitives

void drawLine(lv_layer_t* layer, float x1, float y1, float x2, float y2, lv_color_t c, float width, bool round,
              lv_opa_t opa = LV_OPA_COVER) {
    lv_draw_line_dsc_t d;
    lv_draw_line_dsc_init(&d);
    d.p1.x = Sf(x1);
    d.p1.y = Sf(y1);
    d.p2.x = Sf(x2);
    d.p2.y = Sf(y2);
    d.color = c;
    d.width = S(width) < 1 ? 1 : S(width);
    d.round_start = d.round_end = round;
    d.opa = opa;
    lv_draw_line(layer, &d);
}

void drawArc(lv_layer_t* layer, float cx, float cy, float r, float startDeg, float endDeg, float width, lv_color_t c,
             bool rounded, lv_opa_t opa = LV_OPA_COVER) {
    if (r <= 0) return;
    lv_draw_arc_dsc_t d;
    lv_draw_arc_dsc_init(&d);
    d.center.x = S(cx);
    d.center.y = S(cy);
    d.radius = uint16_t(S(r));
    d.start_angle = startDeg;
    d.end_angle = endDeg;
    d.width = S(width) < 1 ? 1 : S(width);
    d.color = c;
    d.rounded = rounded;
    d.opa = opa;
    lv_draw_arc(layer, &d);
}

// A filled circle: an arc as wide as its radius.
void drawDisc(lv_layer_t* layer, float cx, float cy, float r, lv_color_t c, lv_opa_t opa = LV_OPA_COVER) {
    drawArc(layer, cx, cy, r, 0, 360, r + 1, c, false, opa);
}

void drawTri(lv_layer_t* layer, float x0, float y0, float x1, float y1, float x2, float y2, lv_color_t c,
             lv_opa_t opa = LV_OPA_COVER) {
    lv_draw_triangle_dsc_t d;
    lv_draw_triangle_dsc_init(&d);
    d.p[0].x = Sf(x0);
    d.p[0].y = Sf(y0);
    d.p[1].x = Sf(x1);
    d.p[1].y = Sf(y1);
    d.p[2].x = Sf(x2);
    d.p[2].y = Sf(y2);
    d.color = c;
    d.opa = opa;
    lv_draw_triangle(layer, &d);
}

void drawRectXY(lv_layer_t* layer, float x, float y, float w, float h, float radius, lv_color_t fill, bool filled,
                lv_color_t stroke, float strokeWidth, lv_opa_t opa = LV_OPA_COVER) {
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.radius = S(radius);
    d.bg_color = fill;
    d.bg_opa = filled ? opa : LV_OPA_TRANSP;
    d.border_width = strokeWidth > 0 ? (S(strokeWidth) < 1 ? 1 : S(strokeWidth)) : 0;
    d.border_color = stroke;
    d.border_opa = opa;
    lv_area_t a;
    a.x1 = S(x);
    a.y1 = S(y);
    a.x2 = S(x + w) - 1;
    a.y2 = S(y + h) - 1;
    lv_draw_rect(layer, &d, &a);
}

// Text with its middle at (cx, cy), or its left or right edge there.
void drawText(lv_layer_t* layer, const char* text, const lv_font_t* f, lv_color_t c, float cx, float cy, Align align,
              lv_opa_t opa = LV_OPA_COVER, lv_area_t* extent = nullptr) {
    lv_point_t size;
    lv_text_get_size(&size, text, f, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    lv_area_t a;
    const int32_t x = S(cx), y = S(cy);
    if (align == Align::Left) {
        a.x1 = x;
    } else if (align == Align::Right) {
        a.x1 = x - size.x;
    } else {
        a.x1 = x - size.x / 2;
    }
    a.x2 = a.x1 + size.x;
    a.y1 = y - size.y / 2;
    a.y2 = a.y1 + size.y;
    if (extent) *extent = a;
    if (!layer) return;
    lv_draw_label_dsc_t d;
    lv_draw_label_dsc_init(&d);
    d.text = text;
    d.font = f;
    d.color = c;
    d.opa = opa;
    d.align = LV_TEXT_ALIGN_LEFT;
    lv_draw_label(layer, &d, &a);
}

void unionArea(lv_area_t& a, const lv_area_t& b) {
    if (b.x1 < a.x1) a.x1 = b.x1;
    if (b.y1 < a.y1) a.y1 = b.y1;
    if (b.x2 > a.x2) a.x2 = b.x2;
    if (b.y2 > a.y2) a.y2 = b.y2;
}

lv_area_t boxAround(float cx, float cy, float halfW, float halfH) {
    lv_area_t a;
    a.x1 = S(cx - halfW) - 2;
    a.y1 = S(cy - halfH) - 2;
    a.x2 = S(cx + halfW) + 2;
    a.y2 = S(cy + halfH) + 2;
    return a;
}

// ------------------------------------------------------------- widgets

void drawDial(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    const float cx = widgetX(w), cy = widgetY(w), r = float(w.r);
    const float start = float(w.start), sweep = float(w.sweep);
    const float span = w.max - w.min;
    const lv_color_t fg = col(w.color);
    const float v = it.valid ? clampf(it.value, w.min, w.max) : w.min;
    const float zw = float(w.zoneWidth);
    for (const Zone& z : w.zones) {
        const float a0 = start + sweep * (z.from - w.min) / span, a1 = start + sweep * (z.to - w.min) / span;
        drawArc(layer, cx, cy, r - zw / 2, a0, a1, zw, col(z.color), false);
    }
    const int total = int(w.major) * int(w.minor);
    const float inner = r - zw - 2;
    const float tl = w.ticks ? float(w.tickLen) : 4;
    if (w.arcWidth) drawArc(layer, cx, cy, inner, start, start + sweep, float(w.arcWidth), fg, false);
    const lv_font_t* lf = w.labels ? font(w.labelFont, float(w.labelSize)) : nullptr;
    for (int i = 0; i <= total; i++) {
        const float a = rad(start + sweep * float(i) / float(total));
        const bool isMajor = i % int(w.minor) == 0;
        const float len = isMajor ? float(w.tickLen) : float(w.tickLen) * 0.55f;
        const float ca = cosf(a), sa = sinf(a);
        const lv_opa_t opa = isMajor ? LV_OPA_COVER : LV_OPA_60;
        if (w.ticks && w.tickShape == TickShape::Triangle) {
            float half = len * 0.4f;
            if (half < 2) half = 2;
            const float px = -sa * half, py = ca * half;
            const float bx = cx + ca * inner, by = cy + sa * inner;
            drawTri(layer, bx + px, by + py, bx - px, by - py, cx + ca * (inner - len), cy + sa * (inner - len), fg, opa);
        } else if (w.ticks) {
            const float tw = isMajor ? float(w.tickWidth) : (w.tickWidth > 1 ? float(w.tickWidth) - 1 : 1);
            drawLine(layer, cx + ca * inner, cy + sa * inner, cx + ca * (inner - len), cy + sa * (inner - len), fg, tw,
                     false, opa);
        }
        if (isMajor && lf) {
            const float val = (w.min + span * float(i) / float(total)) / w.labelScale;
            const float lr = inner - tl - float(w.labelSize) * 0.9f;
            drawText(layer, tickText(val).c_str(), lf, fg, cx + ca * lr, cy + sa * lr, Align::Center);
        }
    }
    if (w.needle == NeedleStyle::None) return;
    const float a = rad(start + sweep * (v - w.min) / span);
    const float len = float(w.needleLen), nw = float(w.needleWidth);
    const lv_color_t nc = col(w.needleColor);
    const float ca = cosf(a), sa = sinf(a);
    // Rotates a point in the needle's own frame (x along the needle) into the face.
    auto rx = [&](float x, float y) { return cx + x * ca - y * sa; };
    auto ry = [&](float x, float y) { return cy + x * sa + y * ca; };
    switch (w.needle) {
        case NeedleStyle::Marker:
            drawTri(layer, rx(r + 1, -nw), ry(r + 1, -nw), rx(r + 1, nw), ry(r + 1, nw), rx(r - len, 0), ry(r - len, 0), nc);
            break;
        case NeedleStyle::MarkerOut:
            drawTri(layer, rx(r - len, -nw), ry(r - len, -nw), rx(r - len, nw), ry(r - len, nw), rx(r, 0), ry(r, 0), nc);
            break;
        case NeedleStyle::Line:
            drawLine(layer, cx, cy, cx + ca * len, cy + sa * len, nc, nw, true);
            break;
        default: {
            // A tapered blade from 14 px behind the centre to the tip, as two triangles.
            drawTri(layer, rx(-14, -nw), ry(-14, -nw), rx(len, -1), ry(len, -1), rx(len, 1), ry(len, 1), nc);
            drawTri(layer, rx(-14, -nw), ry(-14, -nw), rx(len, 1), ry(len, 1), rx(-14, nw), ry(-14, nw), nc);
            if (w.needle == NeedleStyle::TaperedCap) {
                drawDisc(layer, cx, cy, nw * 1.8f, nc);
                drawDisc(layer, cx, cy, nw * 0.8f, col(theme.bg));
            }
            break;
        }
    }
}

void drawRing(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    const float cx = widgetX(w), cy = widgetY(w), r = float(w.r), thick = float(w.thickness);
    const float start = float(w.start), sweep = float(w.sweep);
    const float v = it.valid ? clampf(it.value, w.min, w.max) : w.min;
    const float frac = (v - w.min) / (w.max - w.min);
    const float rr = r - thick / 2;
    const lv_color_t trackCol = col(w.trackColor);
    const lv_color_t c = col(applyWarn(w, it.valid, v, zoneColor(w, v, w.color)));
    if (!w.segments) {
        if (w.track) drawArc(layer, cx, cy, rr, start, start + sweep, thick, trackCol, w.rounded);
        if (frac > 0) drawArc(layer, cx, cy, rr, start, start + sweep * frac, thick, c, w.rounded);
        return;
    }
    const float gap = float(w.gap), seg = (sweep - gap * float(w.segments - 1)) / float(w.segments);
    for (int i = 0; i < int(w.segments); i++) {
        const bool lit = (float(i) + 0.5f) / float(w.segments) <= frac;
        if (!lit && !w.track) continue;
        const float a0 = start + float(i) * (seg + gap);
        drawArc(layer, cx, cy, rr, a0, a0 + seg, thick, lit ? c : trackCol, false);
    }
}

void drawBar(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    const bool horiz = w.barStyle == BarStyle::Horizontal;
    const float W = float(w.w), H = float(w.h), cx = widgetX(w), cy = widgetY(w);
    const float v = it.valid ? clampf(it.value, w.min, w.max) : w.min;
    const float frac = (v - w.min) / (w.max - w.min);
    const lv_color_t c = col(applyWarn(w, it.valid, v, zoneColor(w, v, w.color)));
    const lv_color_t trackCol = col(w.trackColor);
    const float cr = float(w.cornerRadius);
    if (!w.rotate) {
        const float x0 = cx - W / 2, y0 = cy - H / 2;
        if (!w.segments) {
            if (w.track) drawRectXY(layer, x0, y0, W, H, cr, trackCol, true, trackCol, 0);
            if (frac > 0) {
                if (horiz) {
                    drawRectXY(layer, x0, y0, W * frac, H, cr, c, true, c, 0);
                } else {
                    drawRectXY(layer, x0, y0 + H * (1 - frac), W, H * frac, cr, c, true, c, 0);
                }
            }
            return;
        }
        const float gap = float(w.gap);
        for (int i = 0; i < int(w.segments); i++) {
            const bool lit = (float(i) + 0.5f) / float(w.segments) <= frac;
            if (!lit && !w.track) continue;
            const lv_color_t sc = lit ? c : trackCol;
            if (horiz) {
                const float sw = (W - gap * float(w.segments - 1)) / float(w.segments);
                drawRectXY(layer, x0 + float(i) * (sw + gap), y0, sw, H, cr, sc, true, sc, 0);
            } else {
                const float sh = (H - gap * float(w.segments - 1)) / float(w.segments);
                drawRectXY(layer, x0, y0 + H - float(i + 1) * sh - float(i) * gap, W, sh, cr, sc, true, sc, 0);
            }
        }
        return;
    }
    // A rotated bar is a thick line along its axis: the long side is the
    // length, the short side the line width.
    const float a = rad(float(w.rotate));
    const float length = horiz ? W : H, width = horiz ? H : W;
    // Along the bar: +x for horizontal, up (-y) for vertical, then rotated.
    const float dx = horiz ? cosf(a) : sinf(a), dy = horiz ? sinf(a) : -cosf(a);
    const float sx = cx - dx * length / 2, sy = cy - dy * length / 2;
    if (!w.segments) {
        if (w.track) drawLine(layer, sx, sy, sx + dx * length, sy + dy * length, trackCol, width, false);
        if (frac > 0) drawLine(layer, sx, sy, sx + dx * length * frac, sy + dy * length * frac, c, width, false);
        return;
    }
    const float gap = float(w.gap), seg = (length - gap * float(w.segments - 1)) / float(w.segments);
    for (int i = 0; i < int(w.segments); i++) {
        const bool lit = (float(i) + 0.5f) / float(w.segments) <= frac;
        if (!lit && !w.track) continue;
        const float p0 = float(i) * (seg + gap);
        drawLine(layer, sx + dx * p0, sy + dy * p0, sx + dx * (p0 + seg), sy + dy * (p0 + seg), lit ? c : trackCol, width,
                 false);
    }
}

// The unit symbol of a number, placed against the text it belongs to.
void drawUnit(lv_layer_t* layer, const Widget& w, lv_color_t c, float cx, float cy, int32_t textW, lv_area_t* extent) {
    if (!w.showUnit || !w.unit || !w.unit->symbol[0]) return;
    const float us = float(w.unitSize), size = float(w.size);
    const lv_font_t* uf = font(Font::Sans, us);
    const float mw = float(textW) / k;  // the text's width in design units
    const float leftEdge = w.align == Align::Center ? cx - mw / 2 : (w.align == Align::Left ? cx : cx - mw);
    const float rightEdge = leftEdge + mw, mid = leftEdge + mw / 2;
    const float dx = float(w.unitDx), dy = float(w.unitDy);
    const lv_opa_t opa = LV_OPA_80;
    switch (w.unitPos) {
        case UnitPos::Below:
            drawText(layer, w.unit->symbol, uf, c, mid + dx, cy + size * 0.55f + us * 0.6f + dy, Align::Center, opa, extent);
            break;
        case UnitPos::Above:
            drawText(layer, w.unit->symbol, uf, c, mid + dx, cy - size * 0.55f - us * 0.6f + dy, Align::Center, opa, extent);
            break;
        case UnitPos::Left:
            // The page puts the symbol's baseline a little below the number's middle.
            drawText(layer, w.unit->symbol, uf, c, leftEdge - 6 + dx, cy + size * 0.3f - us * 0.35f + dy, Align::Right, opa,
                     extent);
            break;
        default:
            drawText(layer, w.unit->symbol, uf, c, rightEdge + 6 + dx, cy + size * 0.3f - us * 0.35f + dy, Align::Left, opa,
                     extent);
            break;
    }
}

void drawNumber(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    if (it.label) return;  // rotated: an LVGL label, updated in update()
    const float cx = widgetX(w), cy = widgetY(w);
    const lv_color_t c = col(applyWarn(w, it.valid, it.value, w.color));
    const lv_font_t* f = font(w.font, float(w.size));
    lv_area_t textExtent;
    drawText(layer, it.text.c_str(), f, c, cx, cy, w.align, LV_OPA_COVER, &textExtent);
    drawUnit(layer, w, c, cx, cy, lv_area_get_width(&textExtent), nullptr);
}

void drawLabel(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    if (it.label || !it.chars.empty()) return;  // rotated or arched: LVGL labels
    drawText(layer, w.text.c_str(), font(w.font, float(w.size)), col(w.color), widgetX(w), widgetY(w), w.align);
}

void drawLight(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    const float cx = widgetX(w), cy = widgetY(w), r = float(w.r);
    const bool visible = litNow(w, it.valid, it.value);
    const lv_color_t c = col(visible ? w.color : w.offColor);
    if (w.shape == LightShape::Text) {
        if (visible) drawText(layer, w.text.c_str(), font(w.font, float(w.size)), c, cx, cy, w.align);
        return;
    }
    if (w.shape == LightShape::Dot) {
        drawDisc(layer, cx, cy, r, c);
    } else if (w.shape == LightShape::Ring) {
        float lw = r / 3;
        if (lw < 3) lw = 3;
        drawArc(layer, cx, cy, r - lw / 2, 0, 360, lw, c, false);
    } else {
        drawRectXY(layer, cx - r, cy - r, 2 * r, 2 * r, r / 4, c, true, c, 0);
    }
    if (!w.text.empty()) {
        const lv_font_t* f = font(w.font, float(w.size));
        lv_point_t sz;
        lv_text_get_size(&sz, w.text.c_str(), f, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        drawText(layer, w.text.c_str(), f, col(theme.fg), cx, cy + r + 4 + float(sz.y) / (2 * k), Align::Center, LV_OPA_80);
    }
}

void drawRim(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    const float cx = widgetX(w), cy = widgetY(w), r = float(w.r), thick = float(w.thickness);
    const lv_color_t c = col(w.color);
    if (w.rimMode == RimMode::Flash) {
        if (it.valid && it.value >= w.from && flashOn(150)) drawArc(layer, cx, cy, r - thick / 2, 0, 360, thick, c, false);
        return;
    }
    if (!it.valid) return;
    const float frac = clampf((it.value - w.from) / (w.to - w.from), 0, 1);
    if (frac > 0) drawArc(layer, cx, cy, r - thick / 2, -90, -90 + 360 * frac, thick, c, false);
}

// A point of a shape, rotated about the widget's centre.
struct Pt {
    float x, y;
};
Pt turn(float cx, float cy, float x, float y, float deg) {
    if (deg == 0) return {cx + x, cy + y};
    const float a = rad(deg), ca = cosf(a), sa = sinf(a);
    return {cx + x * ca - y * sa, cy + x * sa + y * ca};
}

void drawPolygon(lv_layer_t* layer, const std::vector<Pt>& pts, lv_color_t fill, bool filled, lv_color_t stroke,
                 float sw, lv_opa_t opa) {
    if (pts.size() < 3) return;
    if (filled) {
        // A fan from the first point; right for convex shapes, which is what
        // the templates draw. Concave polygons need a proper triangulation.
        for (size_t i = 1; i + 1 < pts.size(); i++) {
            drawTri(layer, pts[0].x, pts[0].y, pts[i].x, pts[i].y, pts[i + 1].x, pts[i + 1].y, fill, opa);
        }
    }
    if (sw > 0) {
        for (size_t i = 0; i < pts.size(); i++) {
            const Pt& a = pts[i];
            const Pt& b = pts[(i + 1) % pts.size()];
            drawLine(layer, a.x, a.y, b.x, b.y, stroke, sw, true, opa);
        }
    }
}

void drawShape(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    const bool lit = litNow(w, it.valid, it.value);
    if (!lit && w.hideWhenOff) return;
    const lv_color_t fill = col(lit ? w.color : w.offColor);
    const lv_color_t stroke = lit ? col(w.strokeColor) : fill;
    const float sw = float(w.strokeWidth);
    const lv_opa_t opa = lv_opa_t(w.opacity * 255 / 100);
    const float cx = widgetX(w), cy = widgetY(w), W = float(w.w), H = float(w.h), rot = float(w.rotate);
    switch (w.shapeKind) {
        case ShapeKind::Line: {
            const Pt a = turn(cx, cy, -W / 2, 0, rot), b = turn(cx, cy, W / 2, 0, rot);
            drawLine(layer, a.x, a.y, b.x, b.y, fill, sw > 0 ? sw : 3, true, opa);
            return;
        }
        case ShapeKind::Ellipse:
            if (rot == 0 || W == H) {
                // A circle when square; otherwise LVGL's nearest, a rounded box.
                drawRectXY(layer, cx - W / 2, cy - H / 2, W, H, (W < H ? W : H) / 2, fill, w.filled, stroke, sw, opa);
                return;
            }
            // fall through: a rotated ellipse is drawn as its box
        case ShapeKind::Rect:
            if (rot == 0) {
                drawRectXY(layer, cx - W / 2, cy - H / 2, W, H, float(w.cornerRadius), fill, w.filled, stroke, sw, opa);
                return;
            }
            drawPolygon(layer,
                        {turn(cx, cy, -W / 2, -H / 2, rot), turn(cx, cy, W / 2, -H / 2, rot), turn(cx, cy, W / 2, H / 2, rot),
                         turn(cx, cy, -W / 2, H / 2, rot)},
                        fill, w.filled, stroke, sw, opa);
            return;
        case ShapeKind::Triangle:
            drawPolygon(layer, {turn(cx, cy, 0, -H / 2, rot), turn(cx, cy, W / 2, H / 2, rot), turn(cx, cy, -W / 2, H / 2, rot)},
                        fill, w.filled, stroke, sw, opa);
            return;
        case ShapeKind::Polygon: {
            std::vector<Pt> pts;
            for (size_t i = 0; i + 1 < w.points.size(); i += 2) pts.push_back(turn(cx, cy, w.points[i], w.points[i + 1], rot));
            drawPolygon(layer, pts, fill, w.filled, stroke, sw, opa);
            return;
        }
    }
}

// ---------------------------------------------------------------- images
// An image widget and a face background are LVGL image objects reading the
// file the hub sent (image_store), decoded by LVGL and kept in its cache.
// They sit above the painter, so an image draws over every vector widget
// whatever its place in the config; the background sits below it.

// "L:/img/<crc>.jpg" for an image the config names, or empty if the hub did
// not send it or the gauge has not kept it.
std::string imageSource(const std::string& name) {
    if (!config || name.empty()) return std::string();
    const ImageRef* ref = findImage(*config, name);
    if (!ref) return std::string();
    const std::string path = image_store::path(ref->crc);
    return path.empty() ? std::string() : "L:" + path;
}

// An image object filling a box, in design units, with the fit the config asks.
lv_obj_t* makeImage(const std::string& src, float cx, float cy, float boxW, float boxH, ImageFit fit, float rotateDeg,
                    uint8_t opacity, bool background) {
    lv_image_header_t header;
    if (lv_image_decoder_get_info(src.c_str(), &header) != LV_RESULT_OK || header.w == 0 || header.h == 0) {
        Serial.printf("image %s could not be read\n", src.c_str());
        return nullptr;
    }
    // Decoded pixels live in RAM, and are decoded again every frame when they
    // do not fit the cache: a board with no PSRAM draws only small images.
    if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) == 0) {
        const uint32_t decoded = uint32_t(header.w) * uint32_t(header.h) * (header.cf == LV_COLOR_FORMAT_RGB565 ? 2u : 4u);
        if (decoded > 60u * 1024u) {
            Serial.printf("image %s is %ux%u: too large to draw on this board\n", src.c_str(), unsigned(header.w),
                          unsigned(header.h));
            return nullptr;
        }
    }
    lv_obj_t* img = lv_image_create(screen);
    lv_image_set_src(img, src.c_str());
    lv_obj_remove_flag(img, LV_OBJ_FLAG_CLICKABLE);
    const int32_t W = S(boxW), H = S(boxH);
    lv_obj_set_size(img, W, H);
    lv_obj_set_pos(img, S(cx) - W / 2, S(cy) - H / 2);
    if (fit == ImageFit::Stretch) {
        lv_image_set_inner_align(img, LV_IMAGE_ALIGN_STRETCH);
    } else {
        const float sx = float(W) / float(header.w), sy = float(H) / float(header.h);
        const float s = fit == ImageFit::Cover ? (sx > sy ? sx : sy) : (sx < sy ? sx : sy);
        lv_image_set_scale(img, uint32_t(s * 256 + 0.5f));
        lv_image_set_inner_align(img, LV_IMAGE_ALIGN_CENTER);  // clipped to the box
    }
    if (rotateDeg != 0) {
        lv_image_set_pivot(img, W / 2, H / 2);
        lv_image_set_rotation(img, int32_t(lroundf(rotateDeg * 10)));
    }
    lv_obj_set_style_image_opa(img, lv_opa_t(opacity * 255 / 100), 0);
    if (background) lv_obj_move_background(img);
    return img;
}

// ----------------------------------------------------------------- paths
// SVG path data (the d attribute) parsed once into an LVGL vector path, in the
// path's own coordinate box, and drawn by ThorVG with the widget's transform.

struct PathReader {
    const char* s;
    bool numberAhead() {
        while (*s == ' ' || *s == ',' || *s == '\t' || *s == '\n') s++;
        return (*s >= '0' && *s <= '9') || *s == '-' || *s == '+' || *s == '.';
    }
    bool number(float& out) {
        if (!numberAhead()) return false;
        char* end = nullptr;
        out = strtof(s, &end);
        if (end == s) return false;
        s = end;
        return true;
    }
    bool flag(bool& out) {
        while (*s == ' ' || *s == ',' || *s == '\t' || *s == '\n') s++;
        if (*s != '0' && *s != '1') return false;
        out = *s == '1';
        s++;
        return true;
    }
};

// A path from SVG path data. Relative commands, shorthand curves and arcs are
// turned into absolute moves, lines, quadratic and cubic curves and arcs.
// Returns false, with the path as far as it got, on data it cannot read.
bool parsePath(const std::string& d, lv_vector_path_t* path) {
    PathReader r{d.c_str()};
    lv_fpoint_t cur = {0, 0}, start = {0, 0}, lastCtrl = {0, 0};
    char lastCmd = 0;
    char cmd = 0;
    for (;;) {
        while (*r.s == ' ' || *r.s == ',' || *r.s == '\t' || *r.s == '\n') r.s++;
        if (!*r.s) return true;
        if (strchr("MmLlHhVvCcSsQqTtAaZz", *r.s)) {
            cmd = *r.s++;
        } else if (!cmd || !r.numberAhead()) {
            return false;
        } else if (cmd == 'M') {
            cmd = 'L';  // further pairs after a move are lines
        } else if (cmd == 'm') {
            cmd = 'l';
        }
        const bool rel = cmd >= 'a' && cmd <= 'z';
        const char C = char(rel ? cmd - 32 : cmd);
        float x, y, x1, y1, x2, y2;
        switch (C) {
            case 'M':
                if (!r.number(x) || !r.number(y)) return false;
                if (rel) { x += cur.x; y += cur.y; }
                cur = start = {x, y};
                lv_vector_path_move_to(path, &cur);
                break;
            case 'L':
                if (!r.number(x) || !r.number(y)) return false;
                if (rel) { x += cur.x; y += cur.y; }
                cur = {x, y};
                lv_vector_path_line_to(path, &cur);
                break;
            case 'H':
                if (!r.number(x)) return false;
                cur.x = rel ? cur.x + x : x;
                lv_vector_path_line_to(path, &cur);
                break;
            case 'V':
                if (!r.number(y)) return false;
                cur.y = rel ? cur.y + y : y;
                lv_vector_path_line_to(path, &cur);
                break;
            case 'C': {
                if (!r.number(x1) || !r.number(y1) || !r.number(x2) || !r.number(y2) || !r.number(x) || !r.number(y)) return false;
                if (rel) { x1 += cur.x; y1 += cur.y; x2 += cur.x; y2 += cur.y; x += cur.x; y += cur.y; }
                const lv_fpoint_t p1 = {x1, y1}, p2 = {x2, y2};
                cur = {x, y};
                lv_vector_path_cubic_to(path, &p1, &p2, &cur);
                lastCtrl = p2;
                break;
            }
            case 'S': {
                if (!r.number(x2) || !r.number(y2) || !r.number(x) || !r.number(y)) return false;
                if (rel) { x2 += cur.x; y2 += cur.y; x += cur.x; y += cur.y; }
                // The first control point mirrors the previous curve's second one.
                const bool chain = lastCmd == 'C' || lastCmd == 'S';
                const lv_fpoint_t p1 = chain ? lv_fpoint_t{2 * cur.x - lastCtrl.x, 2 * cur.y - lastCtrl.y} : cur;
                const lv_fpoint_t p2 = {x2, y2};
                cur = {x, y};
                lv_vector_path_cubic_to(path, &p1, &p2, &cur);
                lastCtrl = p2;
                break;
            }
            case 'Q': {
                if (!r.number(x1) || !r.number(y1) || !r.number(x) || !r.number(y)) return false;
                if (rel) { x1 += cur.x; y1 += cur.y; x += cur.x; y += cur.y; }
                const lv_fpoint_t p1 = {x1, y1};
                cur = {x, y};
                lv_vector_path_quad_to(path, &p1, &cur);
                lastCtrl = p1;
                break;
            }
            case 'T': {
                if (!r.number(x) || !r.number(y)) return false;
                if (rel) { x += cur.x; y += cur.y; }
                const bool chain = lastCmd == 'Q' || lastCmd == 'T';
                const lv_fpoint_t p1 = chain ? lv_fpoint_t{2 * cur.x - lastCtrl.x, 2 * cur.y - lastCtrl.y} : cur;
                cur = {x, y};
                lv_vector_path_quad_to(path, &p1, &cur);
                lastCtrl = p1;
                break;
            }
            case 'A': {
                float rx, ry, rot;
                bool large, sweep;
                if (!r.number(rx) || !r.number(ry) || !r.number(rot) || !r.flag(large) || !r.flag(sweep) || !r.number(x) || !r.number(y)) return false;
                if (rel) { x += cur.x; y += cur.y; }
                cur = {x, y};
                lv_vector_path_arc_to(path, fabsf(rx), fabsf(ry), rot, large, sweep, &cur);
                break;
            }
            case 'Z':
                lv_vector_path_close(path);
                cur = start;
                break;
        }
        lastCmd = C;
    }
}

void drawPath(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    if (!it.vpath) return;
    const bool lit = litNow(w, it.valid, it.value);
    if (!lit && w.hideWhenOff) return;
    const lv_color_t fill = col(lit ? w.color : w.offColor);
    const lv_color_t stroke = lit ? col(w.strokeColor) : fill;
    const lv_opa_t opa = lv_opa_t(w.opacity * 255 / 100);
    const float box = float(w.box), size = float(w.size);
    lv_draw_vector_dsc_t* dsc = lv_draw_vector_dsc_create(layer);
    if (!dsc) return;
    // The path's own box, scaled to `size`, rotated, centred on x, y.
    lv_draw_vector_dsc_identity(dsc);
    lv_draw_vector_dsc_translate(dsc, Sf(widgetX(w)), Sf(widgetY(w)));
    if (w.rotate) lv_draw_vector_dsc_rotate(dsc, float(w.rotate));
    const float scale = Sf(size) / box;
    lv_draw_vector_dsc_scale(dsc, scale, scale);
    lv_draw_vector_dsc_translate(dsc, -box / 2, -box / 2);
    lv_draw_vector_dsc_set_fill_rule(dsc, LV_VECTOR_FILL_EVENODD);
    if (w.filled) {
        lv_draw_vector_dsc_set_fill_color(dsc, fill);
        lv_draw_vector_dsc_set_fill_opa(dsc, opa);
    } else {
        lv_draw_vector_dsc_set_fill_opa(dsc, LV_OPA_TRANSP);
    }
    if (w.strokeWidth > 0) {
        lv_draw_vector_dsc_set_stroke_color(dsc, stroke);
        lv_draw_vector_dsc_set_stroke_opa(dsc, opa);
        lv_draw_vector_dsc_set_stroke_width(dsc, float(w.strokeWidth) / scale);  // in path units
        lv_draw_vector_dsc_set_stroke_join(dsc, LV_VECTOR_STROKE_JOIN_ROUND);
        lv_draw_vector_dsc_set_stroke_cap(dsc, LV_VECTOR_STROKE_CAP_ROUND);
    } else {
        lv_draw_vector_dsc_set_stroke_opa(dsc, LV_OPA_TRANSP);
    }
    lv_draw_vector_dsc_add_path(dsc, it.vpath);
    lv_draw_vector(dsc);
    lv_draw_vector_dsc_delete(dsc);
}

// Images are objects of their own (above), nothing to paint here.
void drawImage(lv_layer_t*, Item&) {}

// The hub's time of day: digits, or hands over twelve marks.
void drawClock(lv_layer_t* layer, Item& it) {
    const Widget& w = *it.w;
    const float cx = widgetX(w), cy = widgetY(w);
    if (w.clockStyle == ClockStyle::Digital) {
        if (it.label) return;  // rotated: an LVGL label, updated in update()
        drawText(layer, it.text.c_str(), font(w.font, float(w.size)), col(w.color), cx, cy, w.align);
        return;
    }
    const float r = float(w.r) * k;
    if (w.ticks) {
        const lv_color_t tc = col(w.trackColor);
        for (int i = 0; i < 12; i++) {
            const float a = float(i) * 30.0f * kPi / 180.0f;
            const float len = (i % 3 == 0 ? 0.14f : 0.07f) * r;
            drawLine(layer, cx + (r - len) * sinf(a), cy - (r - len) * cosf(a), cx + r * sinf(a), cy - r * cosf(a), tc,
                     (i % 3 == 0 ? 2.0f : 1.0f) * float(w.thickness) * k / 2, true);
        }
    }
    if (!it.valid) return;
    const hg::cfg::ClockParts p = hg::cfg::clockParts(uint32_t(it.value));
    const lv_color_t hc = col(w.color);
    const float hw = float(w.thickness) * k;
    auto hand = [&](float deg, float len, float width, lv_color_t c) {
        const float a = deg * kPi / 180.0f;
        drawLine(layer, cx - 0.12f * len * sinf(a), cy + 0.12f * len * cosf(a), cx + len * sinf(a), cy - len * cosf(a), c,
                 width, true);
    };
    hand(hg::cfg::hourHandDegrees(p), r * 0.55f, hw, hc);
    hand(hg::cfg::minuteHandDegrees(p), r * 0.82f, hw * 0.7f, hc);
    if (w.clockSeconds) hand(hg::cfg::secondHandDegrees(p), r * 0.9f, hw > 4 ? 2.0f : hw / 2, col(w.needleColor));
    drawDisc(layer, cx, cy, hw * 0.9f, hc);
}

void drawItem(lv_layer_t* layer, Item& it) {
    switch (it.w->type) {
        case WidgetType::Dial: drawDial(layer, it); break;
        case WidgetType::Ring: drawRing(layer, it); break;
        case WidgetType::Bar: drawBar(layer, it); break;
        case WidgetType::Number: drawNumber(layer, it); break;
        case WidgetType::Label: drawLabel(layer, it); break;
        case WidgetType::Light: drawLight(layer, it); break;
        case WidgetType::Rim: drawRim(layer, it); break;
        case WidgetType::Shape: drawShape(layer, it); break;
        case WidgetType::Path: drawPath(layer, it); break;
        case WidgetType::Image: drawImage(layer, it); break;
        case WidgetType::Clock: drawClock(layer, it); break;
    }
}

void onDraw(lv_event_t* e) {
    lv_layer_t* layer = lv_event_get_layer(e);
    for (Item& it : items) drawItem(layer, it);
}

// ------------------------------------------------------------ geometry

// Where a widget can put pixels, for invalidation. Generous rather than exact.
lv_area_t areaOf(const Item& it) {
    const Widget& w = *it.w;
    const float cx = widgetX(w), cy = widgetY(w);
    switch (w.type) {
        case WidgetType::Dial:
        case WidgetType::Ring:
        case WidgetType::Rim:
            return boxAround(cx, cy, float(w.r) + 2, float(w.r) + 2);
        case WidgetType::Bar:
        case WidgetType::Shape: {
            float hw = float(w.w) / 2, hh = float(w.h) / 2;
            if (w.shapeKind == ShapeKind::Polygon && w.type == WidgetType::Shape) {
                for (size_t i = 0; i + 1 < w.points.size(); i += 2) {
                    if (fabsf(float(w.points[i])) > hw) hw = fabsf(float(w.points[i]));
                    if (fabsf(float(w.points[i + 1])) > hh) hh = fabsf(float(w.points[i + 1]));
                }
            }
            const float d = sqrtf(hw * hw + hh * hh) + float(w.strokeWidth);
            return w.rotate ? boxAround(cx, cy, d, d) : boxAround(cx, cy, hw + float(w.strokeWidth), hh + float(w.strokeWidth));
        }
        case WidgetType::Light: {
            const float r = float(w.r) + 4, cap = w.text.empty() ? 0 : float(w.size) * 1.4f + 4;
            const float hw = w.text.empty() ? r : (r > float(w.text.size()) * float(w.size) * 0.35f ? r : float(w.text.size()) * float(w.size) * 0.35f);
            return boxAround(cx, cy + cap / 2, hw, r + cap / 2);
        }
        case WidgetType::Number: {
            // Measured when the text changes; this is the first guess.
            const float hw = float(w.size) * 0.4f * 5 + float(w.unitSize) * 2 + fabsf(float(w.unitDx));
            const float hh = float(w.size) * 0.7f + float(w.unitSize) * 1.3f + fabsf(float(w.unitDy));
            return boxAround(cx, cy, hw, hh);
        }
        case WidgetType::Label: {
            const float hw = float(w.text.size()) * float(w.size) * 0.4f + 4;
            return boxAround(cx, cy, hw, float(w.size) * 0.7f);
        }
        case WidgetType::Clock:
            if (w.clockStyle == ClockStyle::Analogue) return boxAround(cx, cy, float(w.r) + 2, float(w.r) + 2);
            return boxAround(cx, cy, float(w.size) * 0.4f * 11, float(w.size) * 0.7f);
        case WidgetType::Path:
        case WidgetType::Image:
            return boxAround(cx, cy, float(w.type == WidgetType::Path ? w.size : w.w) / 2 + 2,
                             float(w.type == WidgetType::Path ? w.size : w.h) / 2 + 2);
    }
    return boxAround(cx, cy, 10, 10);
}

// The number's real extent, from its text, so only that is redrawn.
lv_area_t numberArea(const Item& it) {
    const Widget& w = *it.w;
    lv_area_t a;
    drawText(nullptr, it.text.c_str(), font(w.font, float(w.size)), lv_color_black(), widgetX(w), widgetY(w), w.align,
             LV_OPA_COVER, &a);
    lv_area_t u = a;
    drawUnit(nullptr, w, lv_color_black(), widgetX(w), widgetY(w), lv_area_get_width(&a), &u);
    unionArea(a, u);
    a.x1 -= 2;
    a.y1 -= 2;
    a.x2 += 2;
    a.y2 += 2;
    return a;
}

// --------------------------------------------------------- LVGL labels
// Text that LVGL has to transform: rotated numbers and labels, and labels
// laid along an arc, one label per character.

lv_obj_t* makeLabel(const char* text, const lv_font_t* f, lv_color_t c, float cx, float cy, float rotateDeg) {
    lv_obj_t* l = lv_label_create(screen);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, c, 0);
    lv_obj_remove_flag(l, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_update_layout(l);
    const int32_t w = lv_obj_get_width(l), h = lv_obj_get_height(l);
    lv_obj_set_pos(l, S(cx) - w / 2, S(cy) - h / 2);
    lv_obj_set_style_transform_pivot_x(l, w / 2, 0);
    lv_obj_set_style_transform_pivot_y(l, h / 2, 0);
    lv_obj_set_style_transform_rotation(l, int32_t(lroundf(rotateDeg * 10)), 0);
    return l;
}

void buildArcText(Item& it) {
    const Widget& w = *it.w;
    const lv_font_t* f = font(w.font, float(w.size));
    const lv_color_t c = col(w.color);
    const float cx = widgetX(w), cy = widgetY(w), r = float(w.arc);
    const float ang = float(((w.arcAngle % 360) + 360) % 360);
    const bool inside = w.arcSide == ArcSide::Inside ? true : (w.arcSide == ArcSide::Outside ? false : (ang > 0 && ang < 180));
    // Character widths in design units, from the panel font.
    std::vector<std::string> chars;
    std::vector<float> widths;
    float total = 0;
    for (size_t i = 0; i < w.text.size();) {
        // UTF-8: take the whole character.
        size_t n = 1;
        const uint8_t b = uint8_t(w.text[i]);
        if (b >= 0xF0) n = 4; else if (b >= 0xE0) n = 3; else if (b >= 0xC0) n = 2;
        chars.push_back(w.text.substr(i, n));
        lv_point_t sz;
        lv_text_get_size(&sz, chars.back().c_str(), f, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        widths.push_back(float(sz.x) / k);
        total += widths.back();
        i += n;
    }
    const float dir = inside ? -1 : 1;
    float a = rad(ang) - dir * total / (2 * r);
    for (size_t i = 0; i < chars.size(); i++) {
        a += dir * widths[i] / (2 * r);
        const float deg = a * 180.0f / kPi + (inside ? -90 : 90);
        it.chars.push_back(makeLabel(chars[i].c_str(), f, c, cx + r * cosf(a), cy + r * sinf(a), deg));
        a += dir * widths[i] / (2 * r);
    }
}

void fetchThemeColours() { can_task::themeColours(themeCols); }

// What the dynamic widgets look like right now, as a number: when it changes
// the widget is redrawn.
uint32_t lookKey(const Item& it) {
    const Widget& w = *it.w;
    uint32_t key = it.valid ? 1u : 0u;
    switch (w.type) {
        case WidgetType::Dial:
        case WidgetType::Ring:
        case WidgetType::Bar: {
            const float v = it.valid ? clampf(it.value, w.min, w.max) : w.min;
            const float frac = (v - w.min) / (w.max - w.min);
            key |= uint32_t(frac * 720) << 1;  // half a degree on a full dial
            const Color c = applyWarn(w, it.valid, v, zoneColor(w, v, w.color));
            key ^= (c & 0xFFFFFF) << 8;
            break;
        }
        case WidgetType::Number: {
            uint32_t h = 2166136261u;
            for (char ch : it.text) h = (h ^ uint8_t(ch)) * 16777619u;
            key ^= h;
            key ^= (applyWarn(w, it.valid, it.value, w.color) & 0xFFFFFF) << 3;
            break;
        }
        case WidgetType::Light:
        case WidgetType::Shape:
        case WidgetType::Path:
        case WidgetType::Image:
            key |= litNow(w, it.valid, it.value) ? 2u : 0u;
            break;
        case WidgetType::Rim:
            if (w.rimMode == RimMode::Flash) {
                key |= (it.valid && it.value >= w.from && flashOn(150)) ? 2u : 0u;
            } else {
                const float frac = it.valid ? clampf((it.value - w.from) / (w.to - w.from), 0, 1) : 0;
                key |= uint32_t(frac * 720) << 1;
            }
            break;
        case WidgetType::Clock:
            if (w.clockStyle == ClockStyle::Analogue) {
                key |= (it.valid ? uint32_t(it.value) / (w.clockSeconds ? 1u : 60u) : 0u) << 1;
            } else {
                uint32_t h = 2166136261u;
                for (char ch : it.text) h = (h ^ uint8_t(ch)) * 16777619u;
                key ^= h;
            }
            break;
        case WidgetType::Label:
            break;
    }
    return key;
}

}  // namespace

void build(lv_obj_t* parent, const Config& cfg, size_t faceIndex) {
    destroy();
    screen = parent;
    config = &cfg;
    face = faceIndex < cfg.faces.size() ? &cfg.faces[faceIndex] : &cfg.faces[0];
    theme = cfg.theme;
    k = float(display::kSize) / 466.0f;
    fetchThemeColours();

    lv_obj_set_style_bg_color(parent, col(face->bg), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    // The background picture fills the face and sits under everything.
    const std::string bgSrc = imageSource(face->bgImage);
    if (!bgSrc.empty()) {
        background = makeImage(bgSrc, 233, 233, 466, 466, ImageFit::Cover, 0, face->bgImageOpacity, true);
    }

    painter = lv_obj_create(parent);
    lv_obj_remove_style_all(painter);
    lv_obj_set_size(painter, display::kSize, display::kSize);
    lv_obj_set_pos(painter, 0, 0);
    lv_obj_remove_flag(painter, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(painter, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(painter, onDraw, LV_EVENT_DRAW_MAIN, nullptr);

    items.clear();
    items.reserve(face->widgets.size());
    for (const Widget& w : face->widgets) {
        Item it;
        it.w = &w;
        it.area = areaOf(it);
        items.push_back(it);
    }
    for (Item& it : items) {
        const Widget& w = *it.w;
        if (w.type == WidgetType::Label && w.arc > 0) {
            buildArcText(it);
        } else if ((w.type == WidgetType::Label || w.type == WidgetType::Number ||
                    (w.type == WidgetType::Clock && w.clockStyle == ClockStyle::Digital)) && w.rotate) {
            const char* text = w.type == WidgetType::Label ? w.text.c_str() : "---";
            it.label = makeLabel(text, font(w.font, float(w.size)), col(w.color), widgetX(w), widgetY(w), float(w.rotate));
        } else if (w.type == WidgetType::Image) {
            const std::string src = imageSource(w.image);
            if (!src.empty()) {
                it.image = makeImage(src, widgetX(w), widgetY(w), float(w.w), float(w.h), w.fit, float(w.rotate), w.opacity, false);
            }
        } else if (w.type == WidgetType::Path) {
            it.vpath = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);
            if (!parsePath(w.path, it.vpath)) Serial.println("path widget: could not read all of its path data");
        }
    }
    update(now);
}

void destroy() {
    if (painter) lv_obj_delete(painter);
    painter = nullptr;
    if (background) lv_obj_delete(background);
    background = nullptr;
    for (Item& it : items) {
        if (it.label) lv_obj_delete(it.label);
        for (lv_obj_t* c : it.chars) lv_obj_delete(c);
        if (it.image) lv_obj_delete(it.image);
        if (it.vpath) lv_vector_path_delete(it.vpath);
    }
    items.clear();
    face = nullptr;
    config = nullptr;
}

void update(uint32_t nowMs) {
    now = nowMs;
    if (!painter) return;

    // Theme colours the hub changes recolour everything.
    if (now - lastThemeCheckMs >= 500) {
        lastThemeCheckMs = now;
        uint32_t fresh[kMaxThemeColours];
        can_task::themeColours(fresh);
        if (memcmp(fresh, themeCols, sizeof(fresh)) != 0) {
            memcpy(themeCols, fresh, sizeof(fresh));
            lv_obj_set_style_bg_color(screen, col(face->bg), 0);
            for (Item& it : items) {
                if (it.label) lv_obj_set_style_text_color(it.label, col(it.w->color), 0);
                for (lv_obj_t* c : it.chars) lv_obj_set_style_text_color(c, col(it.w->color), 0);
            }
            lv_obj_invalidate(painter);
        }
    }

    const bool wasLive = anyLive;
    anyLive = false;
    introSlots = items.size();
    introSlot = 0;
    for (Item& it : items) {
        const Widget& w = *it.w;
        if (w.type == WidgetType::Label) continue;
        float v = 0;
        bool valid;
        if (w.type == WidgetType::Clock) {
            // The hub's time of day, as seconds into the day (a float holds that exactly).
            uint32_t seconds = 0;
            valid = can_task::localTime(seconds);
            v = float(seconds % 86400u);
        } else {
            valid = readValue(w, v);
        }
        introSlot++;
        if (valid && w.type == WidgetType::Number && w.hold != Hold::None) {
            if (!it.holdSet || (w.hold == Hold::Max ? v > it.hold : v < it.hold)) {
                it.hold = v;
                it.holdSet = true;
            }
            v = it.hold;
        }
        it.valid = valid;
        it.value = v;
        if (w.type == WidgetType::Number || (w.type == WidgetType::Clock && w.clockStyle == ClockStyle::Digital)) {
            std::string text;
            if (w.type == WidgetType::Clock) {
                char buf[16];
                hg::cfg::clockText(valid, uint32_t(v), w.clockSeconds, w.hour12, buf, sizeof(buf));
                text = buf;
            } else {
                text = numberText(w, valid, v);
            }
            if (text != it.text) {
                it.text = text;
                if (it.label) {
                    lv_label_set_text(it.label, text.c_str());
                } else {
                    // Redraw where the old text was and where the new one goes.
                    const lv_area_t next = numberArea(it);
                    lv_area_t both = it.area;
                    unionArea(both, next);
                    it.area = next;
                    lv_obj_invalidate_area(painter, &both);
                    it.key = lookKey(it);
                    continue;
                }
            }
            if (it.label) lv_obj_set_style_text_color(it.label, col(applyWarn(w, valid, v, w.color)), 0);
        }
        const uint32_t key = lookKey(it);
        if (key != it.key) {
            it.key = key;
            if (it.image) {
                const bool show = w.channel < 0 || litNow(w, valid, v);
                if (show) lv_obj_remove_flag(it.image, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(it.image, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_invalidate_area(painter, &it.area);
            }
        }
    }
    (void)wasLive;
}

void setIntro(hg::proto::IntroStyle style, float phase) {
    introOn = true;
    introStyle = style;
    introPhase = phase < 0 ? 0 : (phase > 1 ? 1 : phase);
}

void endIntro() { introOn = false; }

bool demo() { return !everLive; }
bool stale() { return everLive && !anyLive; }

}  // namespace face_renderer
