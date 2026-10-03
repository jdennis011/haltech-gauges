#pragma once

// In-memory form of a gauge configuration: a theme plus a list of faces, each
// a list of widgets. Built from JSON by parseConfig(); see docs/config-schema.md.
//
// Geometry is in pixels on the 466 x 466 round panel, origin top-left, and
// every widget is positioned by its centre. Angles are in degrees, 0 at
// 3 o'clock, increasing clockwise (the convention LVGL and the HTML canvas
// share), so a classic 270 degree dial is start 135, sweep 270.

#include <stdint.h>

#include <string>
#include <vector>

#include "units.h"

namespace hg {
namespace cfg {

constexpr uint8_t kSchemaVersion = 1;
constexpr int kScreenSize = 466;
constexpr size_t kMaxConfigBytes = 32 * 1024;
constexpr size_t kMaxFaces = 8;
constexpr size_t kMaxWidgetsPerFace = 16;
constexpr size_t kMaxZones = 6;
constexpr size_t kMaxTextLength = 32;

typedef uint32_t Color;  // 0xRRGGBB

enum class WidgetType : uint8_t { Dial, Ring, Bar, Number, Label, Light, Rim };
enum class Font : uint8_t { Sans, Condensed, Digital, Mono, Display, Carter, Racing, Trade };
// Marker: arrow at radius r pointing inward; MarkerOut: arrow inside r pointing outward.
enum class NeedleStyle : uint8_t { Line, Tapered, TaperedCap, None, Marker, MarkerOut };
enum class BarStyle : uint8_t { Horizontal, Vertical };
enum class Align : uint8_t { Left, Center, Right };
enum class UnitPos : uint8_t { Right, Below };
enum class Hold : uint8_t { None, Max, Min };
enum class LightShape : uint8_t { Dot, Ring, Square, Text };
enum class TickShape : uint8_t { Line, Triangle };
enum class NumberFormat : uint8_t { Plain, Gear };  // gear: 0 = N, -1 = R, -2 = P
enum class RimMode : uint8_t { Flash, Fill };

struct Theme {
    Color bg = 0x000000;
    Color fg = 0xFFFFFF;
    Color accent = 0xFF3B30;
    Color dim = 0x3A3A3C;
    Color warn = 0xFFCC00;
    Color alert = 0xFF0000;
};

struct ThemePreset {
    const char* name;
    Theme theme;
};
const ThemePreset* themePresets(size_t& count);

// A value range drawn or filled in its own colour (e.g. a redline).
struct Zone {
    float from = 0;
    float to = 0;
    Color color = 0;
};

// Changes a widget's colour when the value crosses a threshold.
struct Warn {
    bool enabled = false;
    bool above = true;  // false: triggers below the threshold
    float threshold = 0;
    Color color = 0xFF0000;
    bool flash = false;
};

struct Widget {
    WidgetType type = WidgetType::Number;

    int16_t channel = -1;               // hg::ChannelId, or -1 for a label
    const DisplayUnit* unit = nullptr;  // conversion from the stored value; null for a label
    float min = 0;                      // in display units
    float max = 100;

    int16_t x = kScreenSize / 2;
    int16_t y = kScreenSize / 2;
    int16_t w = 0;          // bar
    int16_t h = 0;          // bar
    int16_t r = 0;          // dial, ring, rim: outer radius; light: half size
    int16_t thickness = 0;  // ring, rim
    int16_t start = 135;
    int16_t sweep = 270;

    Color color = 0xFFFFFF;
    Color trackColor = 0x3A3A3C;
    bool track = true;
    bool rounded = true;
    uint8_t segments = 0;  // ring, bar: 0 = solid
    uint8_t gap = 3;       // ring: degrees between segments; bar: pixels

    // dial
    uint8_t major = 10;      // divisions between labelled ticks
    uint8_t minor = 5;       // subdivisions per major division
    uint8_t tickLen = 16;
    uint8_t tickWidth = 3;
    TickShape tickShape = TickShape::Line;
    bool ticks = true;      // false: a scale of labels only, e.g. inside a segmented ring
    uint8_t arcWidth = 0;   // thin arc drawn along the sweep at the tick root; 0 = none
    bool labels = true;
    Font labelFont = Font::Sans;
    uint8_t labelSize = 22;
    float labelScale = 1;    // label shows value / labelScale (1000 for RPM x1000)
    NeedleStyle needle = NeedleStyle::Tapered;
    Color needleColor = 0xFF3B30;
    uint8_t needleWidth = 6;
    int16_t needleLen = 0;
    uint8_t zoneWidth = 6;

    // number, label, light caption
    Font font = Font::Sans;
    uint16_t size = 72;
    uint8_t decimals = 0;
    NumberFormat format = NumberFormat::Plain;
    uint8_t pad = 0;        // minimum digits, zero-filled (000 mph)
    bool showUnit = true;
    uint8_t unitSize = 24;
    UnitPos unitPos = UnitPos::Below;
    Align align = Align::Center;
    Hold hold = Hold::None;
    std::string text;
    int16_t rotate = 0;     // label, number, light, bar: degrees clockwise about x, y

    BarStyle barStyle = BarStyle::Horizontal;
    uint8_t cornerRadius = 4;

    LightShape shape = LightShape::Dot;
    Color offColor = 0x3A3A3C;

    // rim
    RimMode rimMode = RimMode::Flash;
    float from = 0;
    float to = 0;

    std::vector<Zone> zones;
    Warn warn;
};

struct Face {
    std::string name;
    Color bg = 0x000000;
    std::vector<Widget> widgets;
};

struct Config {
    std::string name;
    Theme theme;
    std::vector<Face> faces;
};

struct ParseResult {
    bool ok = false;
    std::string error;  // where and why, e.g. "faces[0].widgets[2]: unknown channel 'boost'"
};

// Parses and validates a configuration. On failure `out` is left in an
// unspecified state and must not be used.
ParseResult parseConfig(const uint8_t* json, size_t size, Config& out);

const char* widgetTypeName(WidgetType type);
const char* fontName(Font font);

}  // namespace cfg
}  // namespace hg
