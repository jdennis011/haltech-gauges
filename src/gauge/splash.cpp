#include "splash.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_heap_caps.h>
#include <lvgl.h>
#include <math.h>
#include <string.h>

#include "display.h"
#include "splash_art.h"
#include "svg_path.h"

// Each letter is rasterised once at start, by ThorVG, into its own small
// ARGB8888 image in PSRAM; the entrance and the hand-over then only move and
// blend those images, a few small blits a frame. (Drawing the paths every frame
// instead cost a 75 KB temporary buffer per vector draw per strip on this
// RGB565 panel, and 25 of them: about 6 frames a second.)
namespace splash {
namespace {

using namespace splash_art;

struct Piece {
    lv_obj_t* obj = nullptr;  // an lv_canvas
    void* buf = nullptr;      // its pixels, PSRAM
    int x = 0, y = 0;         // where it sits at rest, panel pixels
    int opa = -1;             // last opacity and y applied, to skip redraws
    int lastY = -1;
};

lv_obj_t* root = nullptr;  // full-screen black
lv_obj_t* arc = nullptr;   // the waiting cue, circling the rim
Piece word[kWordCount];
Piece tag;
bool started = false;   // the entrance runs from the first update()
uint32_t t0 = 0;
uint32_t drawnMs = 0;   // how long the lettering took to rasterise
float fade = 0;         // 0 shown, 1 gone
float k = 1;            // panel pixels per canvas pixel
int lastArcOpa = -1;

float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
// Fast out, settling gently: the SVG's keySplines="0.2 0.8 0.2 1", near enough.
float easeOut(float p) {
    p = 1 - clamp01(p);
    return 1 - p * p * p;
}

float letterPhase(int i, uint32_t elapsed) { return (float(elapsed) - float(i * kLetterStaggerMs)) / float(kLetterMs); }
float tagPhase(uint32_t elapsed) { return (float(elapsed) - float(kTagStartMs)) / float(kTagMs); }

// Rasterises glyphs into a canvas covering `box` (canvas units), padded a pixel.
// Either one Glyph (its own colour) or `count` plain paths in `color`.
bool render(Piece& p, const float* box, const Glyph* glyph, const char* const* paths, uint32_t color, int count) {
    p.x = int(floorf(box[0] * k)) - 1;
    p.y = int(floorf(box[1] * k)) - 1;
    const int w = int(ceilf(box[2] * k)) + 2 - p.x;
    const int h = int(ceilf(box[3] * k)) + 2 - p.y;
    const size_t bytes = size_t(w) * size_t(h) * 4;
    p.buf = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p.buf) return false;
    memset(p.buf, 0, bytes);
    p.obj = lv_canvas_create(root);
    lv_canvas_set_buffer(p.obj, p.buf, w, h, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_remove_flag(p.obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(p.obj, p.x, p.y);
    lv_obj_set_style_image_opa(p.obj, LV_OPA_TRANSP, 0);
    p.opa = 0;
    p.lastY = p.y;

    lv_layer_t layer;
    lv_canvas_init_layer(p.obj, &layer);
    lv_draw_vector_dsc_t* dsc = lv_draw_vector_dsc_create(&layer);
    if (!dsc) {
        lv_canvas_finish_layer(p.obj, &layer);
        return false;
    }
    // Canvas coordinates, scaled to the panel, with the piece's corner at the origin.
    lv_draw_vector_dsc_identity(dsc);
    lv_draw_vector_dsc_translate(dsc, float(-p.x), float(-p.y));
    lv_draw_vector_dsc_scale(dsc, k, k);
    lv_draw_vector_dsc_set_fill_rule(dsc, LV_VECTOR_FILL_NONZERO);
    lv_draw_vector_dsc_set_fill_opa(dsc, LV_OPA_COVER);
    lv_draw_vector_dsc_set_stroke_opa(dsc, LV_OPA_TRANSP);
    bool ok = true;
    for (int i = 0; i < count; i++) {
        lv_vector_path_t* path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);
        if (!svg_path::parse(glyph ? glyph->d : paths[i], path)) ok = false;
        lv_draw_vector_dsc_set_fill_color(dsc, lv_color_hex(glyph ? glyph->color : color));
        lv_draw_vector_dsc_add_path(dsc, path);  // the dsc keeps its own copy
        lv_vector_path_delete(path);
    }
    lv_draw_vector(dsc);
    lv_draw_vector_dsc_delete(dsc);
    lv_canvas_finish_layer(p.obj, &layer);
    return ok;
}

void place(Piece& p, float opa, float rise) {
    if (!p.obj) return;
    const int o = int(255 * clamp01(opa));
    if (o != p.opa) {
        p.opa = o;
        lv_obj_set_style_image_opa(p.obj, lv_opa_t(o), 0);
    }
    const int y = p.y + int(lroundf(rise * k));
    if (y != p.lastY) {
        p.lastY = y;
        lv_obj_set_y(p.obj, y);
    }
}

void setArcOpa(float opa) {
    const int v = int(255 * clamp01(opa));
    if (v == lastArcOpa) return;  // a style change redraws the whole arc
    lastArcOpa = v;
    lv_obj_set_style_arc_opa(arc, lv_opa_t(v), LV_PART_INDICATOR);
}

void freePiece(Piece& p) {
    if (p.buf) heap_caps_free(p.buf);
    p.buf = nullptr;
    p.obj = nullptr;  // deleted with root
}

}  // namespace

void begin() {
    if (root) return;
    k = float(display::kSize) / float(kCanvas);

    root = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(root);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_size(root, display::kSize, display::kSize);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    // Touches stop here: nothing swipes or opens under the splash.
    lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    const uint32_t t = millis();
    bool ok = true;
    for (int i = 0; i < kWordCount; i++) ok = render(word[i], kWord[i].box, &kWord[i], nullptr, 0, 1) && ok;
    ok = render(tag, kTagBox, nullptr, kTag, kTagColor, kTagCount) && ok;
    if (!ok) Serial.println("splash: could not draw all of the lettering");
    drawnMs = millis() - t;

    // The arc: only its indicator is drawn, and only its moving ends are
    // redrawn as it goes round, so the lettering under it stays put.
    arc = lv_arc_create(root);
    lv_obj_remove_style_all(arc);
    const int size = display::scaled(2 * (kArcRadius + kArcWidth));
    lv_obj_set_size(arc, size, size);
    lv_obj_center(arc);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);
    lv_arc_set_rotation(arc, 0);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_angles(arc, 0, kArcSpan);
    lv_obj_set_style_arc_width(arc, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(kArcColor), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, display::scaled(kArcWidth), LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB);
    lastArcOpa = 0;

    started = false;
    fade = 0;

    // The panel comes on at the brightness face_ui keeps (its "gauge"/"bright" preference).
    Preferences p;
    p.begin("gauge", true);
    const uint8_t brightness = p.getUChar("bright", 200);
    p.end();
    display::setBrightness(brightness);
}

void raise() {
    if (root) lv_obj_move_foreground(root);
}

void update(uint32_t now) {
    if (!root) return;
    if (!started) {
        started = true;
        t0 = now;
    }
    const uint32_t elapsed = now - t0;
    const float shown = 1 - fade;
    for (int i = 0; i < kWordCount; i++) {
        const float p = easeOut(letterPhase(i, elapsed));
        place(word[i], p * shown, (1 - p) * kLetterRise);
    }
    place(tag, easeOut(tagPhase(elapsed)) * shown, 0);
    setArcOpa(float(elapsed) / float(kArcFadeMs) * shown);
    const int start = int((elapsed % uint32_t(kArcPeriodMs)) * 360u / uint32_t(kArcPeriodMs));
    lv_arc_set_angles(arc, start, start + kArcSpan);
}

void setFade(float f) {
    if (!root) return;
    f = clamp01(f);
    if (f == fade) return;
    // The black stays: the lettering and the arc fade into it, a few small
    // blits a frame. (Letting the face show through instead cost a full redraw
    // of it under the splash, a quarter of a second, and another at end().)
    fade = f;
}

void end() {
    if (!root) return;
    // Printed here rather than at start, when nothing is listening on USB yet.
    Serial.printf("splash: lettering drawn in %lu ms, entrance from %lu ms after boot, shown %lu ms\n",
                  (unsigned long)drawnMs, (unsigned long)t0, (unsigned long)(millis() - t0));
    lv_obj_delete(root);  // takes the canvases and the arc with it
    root = nullptr;
    arc = nullptr;
    for (int i = 0; i < kWordCount; i++) freePiece(word[i]);
    freePiece(tag);
    started = false;
    fade = 0;
}

bool active() { return root != nullptr; }

}  // namespace splash
