#include "link_icon.h"

namespace link_icon {

namespace {

using can_task::LiveLink;

lv_obj_t* group(lv_obj_t* parent, int size) {
    lv_obj_t* g = lv_obj_create(parent);
    lv_obj_remove_style_all(g);
    lv_obj_set_size(g, size, size);
    lv_obj_set_pos(g, 0, 0);
    lv_obj_remove_flag(g, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(g, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g, LV_OBJ_FLAG_HIDDEN);
    return g;
}

lv_obj_t* block(lv_obj_t* parent, int x, int y, int w, int h, int radius, lv_color_t color) {
    lv_obj_t* b = lv_obj_create(parent);
    lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_bg_color(b, color, 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(b, radius, 0);
    lv_obj_remove_flag(b, LV_OBJ_FLAG_CLICKABLE);
    return b;
}

// Three quarter-circle arcs opening upward from a dot near the bottom.
void makeWifi(lv_obj_t* g, int s, lv_color_t color) {
    const int cx = s / 2, cy = s * 86 / 100;
    const int width = s / 9 < 2 ? 2 : s / 9;
    const int dot = width + width / 2;
    block(g, cx - dot / 2, cy - dot / 2, dot, dot, LV_RADIUS_CIRCLE, color);
    for (int i = 1; i <= 3; i++) {
        const int r = s * (10 + 12 * i) / 100;
        lv_obj_t* a = lv_arc_create(g);
        lv_obj_remove_style_all(a);
        lv_obj_set_size(a, 2 * r, 2 * r);
        lv_obj_set_pos(a, cx - r, cy - r);
        lv_obj_remove_flag(a, LV_OBJ_FLAG_CLICKABLE);
        lv_arc_set_mode(a, LV_ARC_MODE_NORMAL);
        lv_arc_set_rotation(a, 0);
        lv_arc_set_bg_angles(a, 225, 315);  // 12 o'clock, 90 degrees wide
        lv_arc_set_angles(a, 225, 315);
        lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_color(a, color, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(a, width, LV_PART_INDICATOR);
        lv_obj_set_style_arc_rounded(a, true, LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_pad_all(a, 0, LV_PART_KNOB);
    }
}

// A plug: a cable stem, a rounded body drawn as an outline, three contacts.
void makeCan(lv_obj_t* g, int s, lv_color_t color) {
    const int line = s / 9 < 2 ? 2 : s / 9;
    const int bodyW = s * 62 / 100, bodyH = s * 46 / 100;
    const int bodyX = (s - bodyW) / 2, bodyY = s - bodyH - s * 8 / 100;
    block(g, s / 2 - line / 2, s * 8 / 100, line, bodyY - s * 8 / 100 + line, line / 2, color);
    lv_obj_t* body = lv_obj_create(g);
    lv_obj_remove_style_all(body);
    lv_obj_set_pos(body, bodyX, bodyY);
    lv_obj_set_size(body, bodyW, bodyH);
    lv_obj_set_style_border_color(body, color, 0);
    lv_obj_set_style_border_width(body, line, 0);
    lv_obj_set_style_border_opa(body, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(body, line, 0);
    lv_obj_remove_flag(body, LV_OBJ_FLAG_CLICKABLE);
    const int pinW = line, pinH = bodyH * 40 / 100;
    const int gap = (bodyW - 2 * line - 3 * pinW) / 4;
    for (int i = 0; i < 3; i++) {
        const int x = bodyX + line + gap + i * (pinW + gap);
        block(g, x, bodyY + bodyH - line - pinH, pinW, pinH, 0, color);
    }
}

}  // namespace

Icon create(lv_obj_t* parent, int sizePx, lv_color_t color) {
    Icon icon;
    icon.root = lv_obj_create(parent);
    lv_obj_remove_style_all(icon.root);
    lv_obj_set_size(icon.root, sizePx, sizePx);
    lv_obj_remove_flag(icon.root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(icon.root, LV_OBJ_FLAG_SCROLLABLE);
    icon.wifi = group(icon.root, sizePx);
    makeWifi(icon.wifi, sizePx, color);
    icon.can = group(icon.root, sizePx);
    makeCan(icon.can, sizePx, color);
    return icon;
}

void set(Icon& icon, LiveLink link) {
    if (!icon.root) return;
    icon.shown = link;
    if (link == LiveLink::Wifi) {
        lv_obj_remove_flag(icon.wifi, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(icon.wifi, LV_OBJ_FLAG_HIDDEN);
    }
    if (link == LiveLink::Can) {
        lv_obj_remove_flag(icon.can, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(icon.can, LV_OBJ_FLAG_HIDDEN);
    }
}

const char* name(LiveLink link) {
    switch (link) {
        case LiveLink::Can: return "CAN";
        case LiveLink::Wifi: return "Wi-Fi";
        default: return "no data";
    }
}

}  // namespace link_icon
