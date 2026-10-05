#include <ArduinoJson.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "face_model.h"
#include "haltech.h"

namespace hg {
namespace cfg {

namespace {

const ThemePreset kPresets[] = {
    {"white", {0x000000, 0xFFFFFF, 0xFF3B30, 0x3A3A3C, 0xFFCC00, 0xFF0000}},
    {"red", {0x000000, 0xE0281A, 0xFF6A5A, 0x3A0D09, 0xFFB020, 0xFFFFFF}},
    {"amber", {0x000000, 0xFFB000, 0xFF5A00, 0x3D2A00, 0xFFE680, 0xFF2D1A}},
    {"green", {0x000000, 0x39FF88, 0xC8FF3D, 0x0E3B22, 0xFFD23D, 0xFF3B30}},
    {"cyan", {0x000000, 0x2EE6E6, 0x7FF6FF, 0x0B3A3A, 0xFFD23D, 0xFF3B30}},
    {"nfs", {0x000000, 0xFFFFFF, 0x2DB5FF, 0x262626, 0xF6E63B, 0xE0325A}},
};

struct Name {
    const char* name;
    uint8_t value;
};

const Name kWidgetTypes[] = {
    {"dial", uint8_t(WidgetType::Dial)},     {"ring", uint8_t(WidgetType::Ring)},
    {"bar", uint8_t(WidgetType::Bar)},       {"number", uint8_t(WidgetType::Number)},
    {"label", uint8_t(WidgetType::Label)},   {"light", uint8_t(WidgetType::Light)},
    {"rim", uint8_t(WidgetType::Rim)},       {"shape", uint8_t(WidgetType::Shape)},
    {"path", uint8_t(WidgetType::Path)},
};
const Name kFonts[] = {
    {"sans", uint8_t(Font::Sans)},       {"condensed", uint8_t(Font::Condensed)},
    {"digital", uint8_t(Font::Digital)}, {"mono", uint8_t(Font::Mono)},
    {"display", uint8_t(Font::Display)}, {"carter", uint8_t(Font::Carter)},
    {"racing", uint8_t(Font::Racing)},   {"trade", uint8_t(Font::Trade)},
};
const Name kNeedles[] = {
    {"line", uint8_t(NeedleStyle::Line)},
    {"tapered", uint8_t(NeedleStyle::Tapered)},
    {"tapered_cap", uint8_t(NeedleStyle::TaperedCap)},
    {"none", uint8_t(NeedleStyle::None)},
    {"marker", uint8_t(NeedleStyle::Marker)},
    {"marker_out", uint8_t(NeedleStyle::MarkerOut)},
};
const Name kBarStyles[] = {
    {"horizontal", uint8_t(BarStyle::Horizontal)},
    {"vertical", uint8_t(BarStyle::Vertical)},
};
const Name kAligns[] = {
    {"left", uint8_t(Align::Left)},
    {"center", uint8_t(Align::Center)},
    {"right", uint8_t(Align::Right)},
};
const Name kUnitPositions[] = {
    {"right", uint8_t(UnitPos::Right)},
    {"below", uint8_t(UnitPos::Below)},
    {"left", uint8_t(UnitPos::Left)},
    {"above", uint8_t(UnitPos::Above)},
};
const Name kShapeKinds[] = {
    {"rect", uint8_t(ShapeKind::Rect)},         {"ellipse", uint8_t(ShapeKind::Ellipse)},
    {"line", uint8_t(ShapeKind::Line)},         {"triangle", uint8_t(ShapeKind::Triangle)},
    {"polygon", uint8_t(ShapeKind::Polygon)},
};
const Name kHolds[] = {
    {"none", uint8_t(Hold::None)},
    {"max", uint8_t(Hold::Max)},
    {"min", uint8_t(Hold::Min)},
};
const Name kLightShapes[] = {
    {"dot", uint8_t(LightShape::Dot)},
    {"ring", uint8_t(LightShape::Ring)},
    {"square", uint8_t(LightShape::Square)},
    {"text", uint8_t(LightShape::Text)},
};
const Name kTickShapes[] = {
    {"line", uint8_t(TickShape::Line)},
    {"triangle", uint8_t(TickShape::Triangle)},
};
const Name kArcSides[] = {
    {"auto", uint8_t(ArcSide::Auto)},
    {"inside", uint8_t(ArcSide::Inside)},
    {"outside", uint8_t(ArcSide::Outside)},
};
const Name kNumberFormats[] = {
    {"plain", uint8_t(NumberFormat::Plain)},
    {"gear", uint8_t(NumberFormat::Gear)},
};
const Name kRimModes[] = {
    {"flash", uint8_t(RimMode::Flash)},
    {"fill", uint8_t(RimMode::Fill)},
};

template <size_t N>
const char* nameOf(const Name (&table)[N], uint8_t value) {
    for (const Name& n : table) {
        if (n.value == value) return n.name;
    }
    return "";
}

// Collects the first error, prefixed with where in the document it happened.
struct Ctx {
    Theme theme;
    std::string path;
    std::string error;

    bool ok() const { return error.empty(); }

    void fail(const char* fmt, ...) __attribute__((format(printf, 2, 3))) {
        if (!error.empty()) return;
        char buf[160];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        error = path.empty() ? std::string(buf) : path + ": " + buf;
    }
};

bool parseHex(const char* s, Color& out) {
    if (!s || s[0] != '#') return false;
    const size_t len = strlen(s + 1);
    if (len != 6 && len != 3) return false;
    uint32_t v = 0;
    for (size_t i = 0; i < len; i++) {
        const char c = s[1 + i];
        uint32_t d;
        if (c >= '0' && c <= '9') {
            d = uint32_t(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            d = uint32_t(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            d = uint32_t(c - 'A' + 10);
        } else {
            return false;
        }
        v = len == 3 ? (v << 8) | (d * 17) : (v << 4) | d;
    }
    out = v;
    return true;
}

// "themecolour1" to "themecolour32" (or "themecolor"): the index, -1 for a
// different word, -2 for a theme colour number out of range.
int themeColourName(const char* s) {
    const char* rest = nullptr;
    if (strncmp(s, "themecolour", 11) == 0) {
        rest = s + 11;
    } else if (strncmp(s, "themecolor", 10) == 0) {
        rest = s + 10;
    } else {
        return -1;
    }
    if (rest[0] < '1' || rest[0] > '9') return -2;
    int n = 0;
    for (size_t i = 0; rest[i]; i++) {
        if (rest[i] < '0' || rest[i] > '9' || i >= 2) return -2;
        n = n * 10 + (rest[i] - '0');
    }
    return n <= int(kMaxThemeColours) ? n - 1 : -2;
}

// A colour is "#RRGGBB", "#RGB", the name of a theme role, or a hub theme colour.
Color readColor(JsonObjectConst obj, const char* key, Color fallback, Ctx& ctx) {
    JsonVariantConst v = obj[key];
    if (v.isNull()) return fallback;
    const char* s = v.as<const char*>();
    if (!s) {
        ctx.fail("'%s' must be a colour string", key);
        return fallback;
    }
    if (strcmp(s, "bg") == 0) return ctx.theme.bg;
    if (strcmp(s, "fg") == 0) return ctx.theme.fg;
    if (strcmp(s, "accent") == 0) return ctx.theme.accent;
    if (strcmp(s, "dim") == 0) return ctx.theme.dim;
    if (strcmp(s, "warn") == 0) return ctx.theme.warn;
    if (strcmp(s, "alert") == 0) return ctx.theme.alert;
    const int index = themeColourName(s);
    if (index >= 0) return themeColourRef(size_t(index));
    if (index == -2) {
        ctx.fail("'%s': theme colours are themecolour1 to themecolour32", key);
        return fallback;
    }
    Color c;
    if (!parseHex(s, c)) {
        ctx.fail("'%s': '%.20s' is not a colour (use #RRGGBB, a theme role or themecolour1 to 32)", key, s);
        return fallback;
    }
    return c;
}

long readInt(JsonObjectConst obj, const char* key, long fallback, long lo, long hi, Ctx& ctx) {
    JsonVariantConst v = obj[key];
    if (v.isNull()) return fallback;
    if (!v.is<float>()) {
        ctx.fail("'%s' must be a number", key);
        return fallback;
    }
    const double d = v.as<double>();
    if (d < double(lo) || d > double(hi)) {
        ctx.fail("'%s' must be between %ld and %ld", key, lo, hi);
        return fallback;
    }
    return long(d < 0 ? d - 0.5 : d + 0.5);
}

float readFloat(JsonObjectConst obj, const char* key, float fallback, Ctx& ctx) {
    JsonVariantConst v = obj[key];
    if (v.isNull()) return fallback;
    if (!v.is<float>()) {
        ctx.fail("'%s' must be a number", key);
        return fallback;
    }
    return v.as<float>();
}

float requireFloat(JsonObjectConst obj, const char* key, Ctx& ctx) {
    if (obj[key].isNull()) {
        ctx.fail("'%s' is required", key);
        return 0;
    }
    return readFloat(obj, key, 0, ctx);
}

bool readBool(JsonObjectConst obj, const char* key, bool fallback, Ctx& ctx) {
    JsonVariantConst v = obj[key];
    if (v.isNull()) return fallback;
    if (!v.is<bool>()) {
        ctx.fail("'%s' must be true or false", key);
        return fallback;
    }
    return v.as<bool>();
}

template <size_t N>
uint8_t readEnum(JsonObjectConst obj, const char* key, const Name (&table)[N], uint8_t fallback,
                 Ctx& ctx) {
    JsonVariantConst v = obj[key];
    if (v.isNull()) return fallback;
    const char* s = v.as<const char*>();
    if (s) {
        for (const Name& n : table) {
            if (strcmp(n.name, s) == 0) return n.value;
        }
    }
    ctx.fail("'%s': unknown value '%.20s'", key, s ? s : "");
    return fallback;
}

std::string readText(JsonObjectConst obj, const char* key, Ctx& ctx) {
    JsonVariantConst v = obj[key];
    if (v.isNull()) return std::string();
    const char* s = v.as<const char*>();
    if (!s) {
        ctx.fail("'%s' must be text", key);
        return std::string();
    }
    if (strlen(s) > kMaxTextLength) {
        ctx.fail("'%s' is longer than %u characters", key, unsigned(kMaxTextLength));
        return std::string();
    }
    return std::string(s);
}

void readChannel(JsonObjectConst obj, Widget& w, Ctx& ctx) {
    const char* name = obj["channel"].as<const char*>();
    if (!name) {
        ctx.fail("'channel' is required");
        return;
    }
    const int id = findChannel(name);
    if (id < 0) {
        ctx.fail("unknown channel '%.40s'", name);
        return;
    }
    w.channel = int16_t(id);

    const Unit base = channelDef(ChannelId(id)).unit;
    JsonVariantConst unit = obj["unit"];
    const char* unitName = unit.isNull() ? nullptr : unit.as<const char*>();
    if (!unit.isNull() && !unitName) {
        ctx.fail("'unit' must be text");
        return;
    }
    w.unit = findDisplayUnit(base, unitName);
    if (!w.unit) ctx.fail("unit '%.20s' is not available for channel '%.40s'", unitName, name);
}

void readRange(JsonObjectConst obj, Widget& w, Ctx& ctx) {
    w.min = requireFloat(obj, "min", ctx);
    w.max = requireFloat(obj, "max", ctx);
    if (ctx.ok() && !(w.min < w.max)) ctx.fail("'min' must be less than 'max'");
}

void readArc(JsonObjectConst obj, Widget& w, int minSweep, Ctx& ctx) {
    w.start = int16_t(readInt(obj, "start", 135, -360, 360, ctx));
    w.start = int16_t(((w.start % 360) + 360) % 360);
    w.sweep = int16_t(readInt(obj, "sweep", 270, minSweep, 360, ctx));
}

void readZones(JsonObjectConst obj, Widget& w, Ctx& ctx) {
    JsonVariantConst v = obj["zones"];
    if (v.isNull()) return;
    JsonArrayConst zones = v.as<JsonArrayConst>();
    if (zones.isNull()) {
        ctx.fail("'zones' must be a list");
        return;
    }
    if (zones.size() > kMaxZones) {
        ctx.fail("more than %u zones", unsigned(kMaxZones));
        return;
    }
    for (JsonVariantConst item : zones) {
        JsonObjectConst zo = item.as<JsonObjectConst>();
        if (zo.isNull()) {
            ctx.fail("each zone must be an object");
            return;
        }
        Zone z;
        z.from = requireFloat(zo, "from", ctx);
        z.to = requireFloat(zo, "to", ctx);
        z.color = readColor(zo, "color", ctx.theme.alert, ctx);
        if (ctx.ok() && !(z.from < z.to)) ctx.fail("zone 'from' must be less than 'to'");
        if (!ctx.ok()) return;
        w.zones.push_back(z);
    }
}

// {"above": n} or {"below": n}, with optional colour and flash.
void readThreshold(JsonObjectConst obj, const char* key, Warn& warn, Ctx& ctx) {
    JsonVariantConst v = obj[key];
    if (v.isNull()) return;
    JsonObjectConst wo = v.as<JsonObjectConst>();
    if (wo.isNull()) {
        ctx.fail("'%s' must be an object", key);
        return;
    }
    const bool hasAbove = !wo["above"].isNull();
    const bool hasBelow = !wo["below"].isNull();
    if (hasAbove == hasBelow) {
        ctx.fail("'%s' needs exactly one of 'above' or 'below'", key);
        return;
    }
    warn.enabled = true;
    warn.above = hasAbove;
    warn.threshold = readFloat(wo, hasAbove ? "above" : "below", 0, ctx);
    warn.color = readColor(wo, "color", ctx.theme.alert, ctx);
    warn.flash = readBool(wo, "flash", false, ctx);
}

void readTextStyle(JsonObjectConst obj, Widget& w, uint16_t defaultSize, Ctx& ctx) {
    w.font = Font(readEnum(obj, "font", kFonts, uint8_t(Font::Sans), ctx));
    w.size = uint16_t(readInt(obj, "size", defaultSize, 8, 200, ctx));
    w.align = Align(readEnum(obj, "align", kAligns, uint8_t(Align::Center), ctx));
}

// Fill and outline of a shape or path.
void readFill(JsonObjectConst obj, Widget& w, Ctx& ctx) {
    w.filled = readBool(obj, "filled", true, ctx);
    w.strokeColor = readColor(obj, "strokeColor", w.color, ctx);
    w.strokeWidth = uint8_t(readInt(obj, "strokeWidth", w.filled ? 0 : 3, 0, 40, ctx));
    w.opacity = uint8_t(readInt(obj, "opacity", 100, 0, 100, ctx));
    w.rotate = int16_t(readInt(obj, "rotate", 0, -180, 180, ctx));
}

// With a channel, a shape or path is lit while its "on" condition holds, as a light is.
void readCondition(JsonObjectConst obj, Widget& w, Ctx& ctx) {
    w.offColor = readColor(obj, "offColor", ctx.theme.dim, ctx);
    w.hideWhenOff = readBool(obj, "hideWhenOff", false, ctx);
    if (w.channel < 0) return;
    w.warn.enabled = true;
    w.warn.above = true;
    w.warn.threshold = 0.5f;
    readThreshold(obj, "on", w.warn, ctx);
    w.warn.flash = readBool(obj, "flash", w.warn.flash, ctx);
}

void readPoints(JsonObjectConst obj, Widget& w, Ctx& ctx) {
    JsonArrayConst a = obj["points"].as<JsonArrayConst>();
    if (a.isNull() || a.size() < 6 || a.size() % 2 != 0 || a.size() > 2 * kMaxPolygonPoints) {
        ctx.fail("'points' must list 3 to %u x, y pairs", unsigned(kMaxPolygonPoints));
        return;
    }
    for (JsonVariantConst v : a) {
        if (!v.is<float>()) {
            ctx.fail("'points' must be numbers");
            return;
        }
        const long n = v.as<long>();
        if (n < -kScreenSize || n > kScreenSize) {
            ctx.fail("'points' must be within %d of the centre", kScreenSize);
            return;
        }
        w.points.push_back(int16_t(n));
    }
}

// SVG path data: move, line, curve, arc and close commands with numbers.
std::string readPathData(JsonObjectConst obj, Ctx& ctx) {
    const char* s = obj["d"].as<const char*>();
    if (!s || !s[0]) {
        ctx.fail("'d' is required: the path data of an SVG path");
        return std::string();
    }
    const size_t len = strlen(s);
    if (len > kMaxPathLength) {
        ctx.fail("'d' is longer than %u characters", unsigned(kMaxPathLength));
        return std::string();
    }
    for (size_t i = 0; i < len; i++) {
        if (!strchr("MmLlHhVvCcSsQqTtAaZz0123456789 .,+-eE\t\n", s[i])) {
            ctx.fail("'d' has a character that is not SVG path data");
            return std::string();
        }
    }
    return std::string(s);
}

void parseWidget(JsonObjectConst obj, Widget& w, Ctx& ctx) {
    const char* typeName = obj["type"].as<const char*>();
    if (!typeName) {
        ctx.fail("'type' is required");
        return;
    }
    bool known = false;
    for (const Name& n : kWidgetTypes) {
        if (strcmp(n.name, typeName) == 0) {
            w.type = WidgetType(n.value);
            known = true;
        }
    }
    if (!known) {
        ctx.fail("unknown widget type '%.20s'", typeName);
        return;
    }

    const Theme& t = ctx.theme;
    w.x = int16_t(readInt(obj, "x", kScreenSize / 2, 0, kScreenSize, ctx));
    w.y = int16_t(readInt(obj, "y", kScreenSize / 2, 0, kScreenSize, ctx));
    w.color = readColor(obj, "color", t.fg, ctx);
    // A shape or path is plain decoration unless it names a channel.
    const bool decoration = w.type == WidgetType::Shape || w.type == WidgetType::Path;
    if (w.type != WidgetType::Label && !(decoration && obj["channel"].isNull())) readChannel(obj, w, ctx);
    if (!ctx.ok()) return;

    switch (w.type) {
        case WidgetType::Dial:
            readRange(obj, w, ctx);
            readArc(obj, w, 30, ctx);
            w.r = int16_t(readInt(obj, "r", 220, 40, kScreenSize / 2, ctx));
            w.major = uint8_t(readInt(obj, "major", 10, 1, 20, ctx));
            w.minor = uint8_t(readInt(obj, "minor", 5, 1, 10, ctx));
            w.tickLen = uint8_t(readInt(obj, "tickLen", 16, 2, 60, ctx));
            w.tickWidth = uint8_t(readInt(obj, "tickWidth", 3, 1, 10, ctx));
            w.ticks = readBool(obj, "ticks", true, ctx);
            w.tickShape = TickShape(readEnum(obj, "tickShape", kTickShapes, uint8_t(TickShape::Line), ctx));
            w.arcWidth = uint8_t(readInt(obj, "arcWidth", 0, 0, 20, ctx));
            w.labels = readBool(obj, "labels", true, ctx);
            w.labelFont = Font(readEnum(obj, "labelFont", kFonts, uint8_t(Font::Sans), ctx));
            w.labelSize = uint8_t(readInt(obj, "labelSize", 22, 8, 60, ctx));
            w.labelScale = readFloat(obj, "labelScale", 1, ctx);
            if (ctx.ok() && !(w.labelScale > 0)) ctx.fail("'labelScale' must be greater than 0");
            w.needle = NeedleStyle(readEnum(obj, "needle", kNeedles, uint8_t(NeedleStyle::Tapered), ctx));
            w.needleColor = readColor(obj, "needleColor", t.accent, ctx);
            w.needleWidth = uint8_t(readInt(obj, "needleWidth", 6, 1, 20, ctx));
            // A marker is short by default: an arrow from the rim pointing at the value.
            w.needleLen = int16_t(readInt(obj, "needleLen",
                                          (w.needle == NeedleStyle::Marker || w.needle == NeedleStyle::MarkerOut) ? 28 : (w.r > 34 ? w.r - 24 : 10), 10, w.r, ctx));
            w.zoneWidth = uint8_t(readInt(obj, "zoneWidth", 6, 1, 40, ctx));
            readZones(obj, w, ctx);
            break;

        case WidgetType::Ring:
            readRange(obj, w, ctx);
            readArc(obj, w, 10, ctx);
            w.r = int16_t(readInt(obj, "r", 225, 40, kScreenSize / 2, ctx));
            w.thickness = int16_t(readInt(obj, "thickness", 20, 2, 120, ctx));
            if (ctx.ok() && w.thickness >= w.r) ctx.fail("'thickness' must be less than 'r'");
            w.rounded = readBool(obj, "rounded", true, ctx);
            w.track = readBool(obj, "track", true, ctx);
            w.trackColor = readColor(obj, "trackColor", t.dim, ctx);
            w.segments = uint8_t(readInt(obj, "segments", 0, 0, 60, ctx));
            w.gap = uint8_t(readInt(obj, "gap", 3, 0, 20, ctx));
            readZones(obj, w, ctx);
            break;

        case WidgetType::Bar:
            readRange(obj, w, ctx);
            w.barStyle = BarStyle(readEnum(obj, "style", kBarStyles, uint8_t(BarStyle::Horizontal), ctx));
            w.w = int16_t(readInt(obj, "w", w.barStyle == BarStyle::Horizontal ? 260 : 28, 4, kScreenSize, ctx));
            w.h = int16_t(readInt(obj, "h", w.barStyle == BarStyle::Horizontal ? 28 : 260, 4, kScreenSize, ctx));
            w.track = readBool(obj, "track", true, ctx);
            w.trackColor = readColor(obj, "trackColor", t.dim, ctx);
            w.segments = uint8_t(readInt(obj, "segments", 0, 0, 60, ctx));
            w.gap = uint8_t(readInt(obj, "gap", 3, 0, 20, ctx));
            w.cornerRadius = uint8_t(readInt(obj, "cornerRadius", 4, 0, 60, ctx));
            w.rotate = int16_t(readInt(obj, "rotate", 0, -180, 180, ctx));
            readZones(obj, w, ctx);
            break;

        case WidgetType::Number:
            readTextStyle(obj, w, 72, ctx);
            w.decimals = uint8_t(readInt(obj, "decimals", 0, 0, 3, ctx));
            w.format = NumberFormat(readEnum(obj, "format", kNumberFormats, uint8_t(NumberFormat::Plain), ctx));
            w.pad = uint8_t(readInt(obj, "pad", 0, 0, 6, ctx));
            w.showUnit = readBool(obj, "showUnit", true, ctx);
            w.unitSize = uint8_t(readInt(obj, "unitSize", w.size / 3 < 12 ? 12 : w.size / 3, 8, 80, ctx));
            w.unitPos = UnitPos(readEnum(obj, "unitPos", kUnitPositions, uint8_t(UnitPos::Below), ctx));
            w.unitDx = int16_t(readInt(obj, "unitDx", 0, -200, 200, ctx));
            w.unitDy = int16_t(readInt(obj, "unitDy", 0, -200, 200, ctx));
            w.hold = Hold(readEnum(obj, "hold", kHolds, uint8_t(Hold::None), ctx));
            w.rotate = int16_t(readInt(obj, "rotate", 0, -180, 180, ctx));
            break;

        case WidgetType::Label:
            readTextStyle(obj, w, 24, ctx);
            w.text = readText(obj, "text", ctx);
            if (ctx.ok() && w.text.empty()) ctx.fail("'text' is required");
            w.rotate = int16_t(readInt(obj, "rotate", 0, -180, 180, ctx));
            w.arc = int16_t(readInt(obj, "arc", 0, 0, kScreenSize / 2, ctx));
            w.arcAngle = int16_t(readInt(obj, "arcAngle", 270, -360, 360, ctx));
            w.arcAngle = int16_t(((w.arcAngle % 360) + 360) % 360);
            w.arcSide = ArcSide(readEnum(obj, "arcSide", kArcSides, uint8_t(ArcSide::Auto), ctx));
            break;

        case WidgetType::Light:
            readTextStyle(obj, w, 18, ctx);
            w.r = int16_t(readInt(obj, "r", 14, 4, 100, ctx));
            w.shape = LightShape(readEnum(obj, "shape", kLightShapes, uint8_t(LightShape::Dot), ctx));
            w.color = readColor(obj, "color", t.alert, ctx);
            w.offColor = readColor(obj, "offColor", t.dim, ctx);
            w.text = readText(obj, "text", ctx);
            if (ctx.ok() && w.shape == LightShape::Text && w.text.empty()) {
                ctx.fail("'text' is required for shape 'text'");
            }
            // Lit when the value crosses the threshold; by default, when a flag is set.
            w.warn.enabled = true;
            w.warn.above = true;
            w.warn.threshold = 0.5f;
            readThreshold(obj, "on", w.warn, ctx);
            w.warn.flash = readBool(obj, "flash", w.warn.flash, ctx);
            w.rotate = int16_t(readInt(obj, "rotate", 0, -180, 180, ctx));
            break;

        case WidgetType::Rim:
            w.rimMode = RimMode(readEnum(obj, "mode", kRimModes, uint8_t(RimMode::Flash), ctx));
            w.r = int16_t(readInt(obj, "r", kScreenSize / 2, 40, kScreenSize / 2, ctx));
            w.thickness = int16_t(readInt(obj, "thickness", 12, 2, 60, ctx));
            w.color = readColor(obj, "color", t.alert, ctx);
            w.from = requireFloat(obj, "from", ctx);
            if (w.rimMode == RimMode::Fill) {
                w.to = requireFloat(obj, "to", ctx);
                if (ctx.ok() && !(w.from < w.to)) ctx.fail("'from' must be less than 'to'");
            } else {
                w.to = w.from;
            }
            break;

        case WidgetType::Shape:
            w.shapeKind = ShapeKind(readEnum(obj, "shape", kShapeKinds, uint8_t(ShapeKind::Rect), ctx));
            w.w = int16_t(readInt(obj, "w", 100, 1, kScreenSize, ctx));
            w.h = int16_t(readInt(obj, "h", 60, 1, kScreenSize, ctx));
            w.cornerRadius = uint8_t(readInt(obj, "cornerRadius", 0, 0, 233, ctx));
            readFill(obj, w, ctx);
            if (w.shapeKind == ShapeKind::Polygon) readPoints(obj, w, ctx);
            readCondition(obj, w, ctx);
            break;

        case WidgetType::Path:
            w.path = readPathData(obj, ctx);
            w.box = int16_t(readInt(obj, "box", 24, 1, 2000, ctx));
            w.size = uint16_t(readInt(obj, "size", 48, 4, kScreenSize, ctx));
            readFill(obj, w, ctx);
            readCondition(obj, w, ctx);
            break;
    }

    const bool usesOn = w.type == WidgetType::Light || w.type == WidgetType::Shape || w.type == WidgetType::Path;
    if (!usesOn) readThreshold(obj, "warn", w.warn, ctx);
}

void parseTheme(JsonVariantConst v, Ctx& ctx) {
    if (v.isNull()) return;  // default theme
    if (const char* name = v.as<const char*>()) {
        for (const ThemePreset& p : kPresets) {
            if (strcmp(p.name, name) == 0) {
                ctx.theme = p.theme;
                return;
            }
        }
        ctx.fail("unknown theme '%.20s'", name);
        return;
    }
    JsonObjectConst obj = v.as<JsonObjectConst>();
    if (obj.isNull()) {
        ctx.fail("'theme' must be a preset name or an object of colours");
        return;
    }
    // Roles are not usable inside the theme itself: each entry is a hex colour or a
    // hub theme colour.
    struct Role {
        const char* key;
        Color* slot;
    };
    Theme theme;
    const Role roles[] = {{"bg", &theme.bg},         {"fg", &theme.fg},     {"accent", &theme.accent},
                          {"dim", &theme.dim},       {"warn", &theme.warn}, {"alert", &theme.alert}};
    for (const Role& r : roles) {
        JsonVariantConst c = obj[r.key];
        if (c.isNull()) continue;
        const char* s = c.as<const char*>();
        const int index = s ? themeColourName(s) : -1;
        if (index >= 0) {
            *r.slot = themeColourRef(size_t(index));
            continue;
        }
        if (!parseHex(s, *r.slot)) {
            ctx.fail("theme '%s' must be a #RRGGBB colour or themecolour1 to 32", r.key);
            return;
        }
    }
    ctx.theme = theme;
}

}  // namespace

const ThemePreset* themePresets(size_t& count) {
    count = sizeof(kPresets) / sizeof(kPresets[0]);
    return kPresets;
}

ThemeColours::ThemeColours() {
    static const Color kValues[kMinThemeColours] = {0xFFFFFF, 0xFF8C00, 0xFF3B30, 0xFFCC00,
                                                    0x30D158, 0x0A84FF, 0x8E8E93, 0x000000};
    static const char* const kNames[kMinThemeColours] = {"White", "Orange", "Red",  "Yellow",
                                                         "Green", "Blue",   "Grey", "Black"};
    for (size_t i = 0; i < kMaxThemeColours; i++) {
        value[i] = i < kMinThemeColours ? kValues[i] : kUnsetThemeColour;
        name[i] = i < kMinThemeColours ? kNames[i] : "";
    }
}

ParseResult parseThemeColours(const uint8_t* json, size_t size, ThemeColours& out) {
    ParseResult result;
    JsonDocument doc;
    if (deserializeJson(doc, json, size)) {
        result.error = "not valid JSON";
        return result;
    }
    JsonVariantConst root = doc.as<JsonVariantConst>();
    JsonArrayConst list = root.is<JsonObjectConst>() ? root["colours"].as<JsonArrayConst>()
                                                      : root.as<JsonArrayConst>();
    if (list.isNull() || list.size() < kMinThemeColours || list.size() > kMaxThemeColours) {
        result.error = "theme colours must be a list of 8 to 32";
        return result;
    }
    ThemeColours tc;
    tc.count = list.size();
    char buf[64];
    size_t i = 0;
    for (JsonVariantConst item : list) {
        const char* value = item["value"].as<const char*>();
        if (!parseHex(value, tc.value[i])) {
            snprintf(buf, sizeof(buf), "theme colour %u: 'value' must look like #FF8C00", unsigned(i + 1));
            result.error = buf;
            return result;
        }
        const char* name = item["name"] | "";
        if (strlen(name) > kMaxThemeColourName) {
            snprintf(buf, sizeof(buf), "theme colour %u: 'name' is longer than 16 characters", unsigned(i + 1));
            result.error = buf;
            return result;
        }
        tc.name[i] = name;
        i++;
    }
    out = tc;
    result.ok = true;
    return result;
}

void formatColor(Color c, char out[8]) { snprintf(out, 8, "#%06lX", (unsigned long)(c & 0xFFFFFF)); }

const char* widgetTypeName(WidgetType type) { return nameOf(kWidgetTypes, uint8_t(type)); }
const char* fontName(Font font) { return nameOf(kFonts, uint8_t(font)); }

ParseResult parseConfig(const uint8_t* json, size_t size, Config& out) {
    ParseResult result;
    Ctx ctx;
    out = Config();

    if (size > kMaxConfigBytes) {
        result.error = "config is larger than the 32 KB limit";
        return result;
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, json, size);
    if (err) {
        result.error = std::string("not valid JSON: ") + err.c_str();
        return result;
    }
    JsonObjectConst root = doc.as<JsonObjectConst>();
    if (root.isNull()) {
        result.error = "config must be a JSON object";
        return result;
    }

    if (!root["schema"].is<int>()) {
        ctx.fail("'schema' is required");
    } else {
        const int schema = root["schema"].as<int>();
        if (schema > kSchemaVersion) {
            ctx.fail("config schema %d is newer than this firmware supports (%d)", schema,
                     int(kSchemaVersion));
        } else if (schema < 1) {
            ctx.fail("'schema' must be 1 or higher");
        }
    }

    if (ctx.ok()) out.name = readText(root, "name", ctx);
    if (ctx.ok()) parseTheme(root["theme"], ctx);
    out.theme = ctx.theme;

    JsonArrayConst faces = root["faces"].as<JsonArrayConst>();
    if (ctx.ok()) {
        if (faces.isNull() || faces.size() == 0) {
            ctx.fail("'faces' must list at least one face");
        } else if (faces.size() > kMaxFaces) {
            ctx.fail("more than %u faces", unsigned(kMaxFaces));
        }
    }

    char path[48];
    size_t fi = 0;
    for (JsonVariantConst fv : faces) {
        if (!ctx.ok()) break;
        snprintf(path, sizeof(path), "faces[%u]", unsigned(fi));
        ctx.path = path;

        JsonObjectConst fo = fv.as<JsonObjectConst>();
        if (fo.isNull()) {
            ctx.fail("must be an object");
            break;
        }
        Face face;
        face.name = readText(fo, "name", ctx);
        face.bg = readColor(fo, "bg", ctx.theme.bg, ctx);

        JsonArrayConst widgets = fo["widgets"].as<JsonArrayConst>();
        if (ctx.ok()) {
            if (widgets.isNull() || widgets.size() == 0) {
                ctx.fail("'widgets' must list at least one widget");
            } else if (widgets.size() > kMaxWidgetsPerFace) {
                ctx.fail("more than %u widgets", unsigned(kMaxWidgetsPerFace));
            }
        }

        size_t wi = 0;
        for (JsonVariantConst wv : widgets) {
            if (!ctx.ok()) break;
            snprintf(path, sizeof(path), "faces[%u].widgets[%u]", unsigned(fi), unsigned(wi));
            ctx.path = path;

            JsonObjectConst wo = wv.as<JsonObjectConst>();
            if (wo.isNull()) {
                ctx.fail("must be an object");
                break;
            }
            Widget widget;
            parseWidget(wo, widget, ctx);
            if (ctx.ok()) face.widgets.push_back(std::move(widget));
            wi++;
        }
        if (ctx.ok()) out.faces.push_back(std::move(face));
        fi++;
    }

    result.ok = ctx.ok();
    result.error = ctx.error;
    return result;
}

}  // namespace cfg
}  // namespace hg
