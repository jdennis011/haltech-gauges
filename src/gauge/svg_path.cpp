#include "svg_path.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace svg_path {
namespace {

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
}  // namespace

bool parse(const char* d, lv_vector_path_t* path) {
    PathReader r{d};
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

}  // namespace svg_path
