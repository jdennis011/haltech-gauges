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
constexpr size_t kMaxPathLength = 1024;   // SVG path data of a path widget
constexpr size_t kMaxPolygonPoints = 16;
// Images: kept in the hub's library by name (the rules of a config name) and
// used by name in a config. A config uses at most this many different ones.
constexpr size_t kMaxImageName = 32;
constexpr size_t kMaxImagesPerConfig = 16;
// The hub's "images" map is added to a config on its way to a gauge, so a
// config at the 32 KB limit still fits with it.
constexpr size_t kMaxImageMapBytes = 1024;

typedef uint32_t Color;  // 0xRRGGBB, or a reference to one of the hub's theme colours

// Theme colours: 8 to 32 colours kept on the hub and shared by every config and
// gauge. A config names them "themecolour1" to "themecolour32"; the parser keeps
// the reference, and whatever draws the face looks it up with resolveColor(),
// so changing one on the hub recolours every widget that uses it. The hub
// always has the first eight, and colours are added and removed only at the
// end, so a number always means the same slot.
constexpr size_t kMinThemeColours = 8;
constexpr size_t kMaxThemeColours = 32;
constexpr size_t kMaxThemeColourName = 16;
constexpr Color kUnsetThemeColour = 0xFFFFFF;  // a slot the hub has not filled is drawn white
constexpr Color kThemeColourRef = 0x80000000u;
constexpr Color themeColourRef(size_t index) { return kThemeColourRef | Color(index); }
constexpr bool isThemeColourRef(Color c) { return (c & kThemeColourRef) != 0; }
constexpr size_t themeColourIndex(Color c) { return size_t(c & 0xFF); }
// The colour to draw: a theme colour reference looked up in the hub's table of
// kMaxThemeColours, anything else as it is.
inline Color resolveColor(Color c, const Color themeColours[kMaxThemeColours]) {
    if (!isThemeColourRef(c)) return c;
    const size_t i = themeColourIndex(c);
    return i < kMaxThemeColours ? themeColours[i] : kUnsetThemeColour;
}

struct ThemeColours {
    size_t count = kMinThemeColours;
    Color value[kMaxThemeColours];  // past `count`: kUnsetThemeColour
    std::string name[kMaxThemeColours];
    ThemeColours();  // the eight defaults: white, orange, red, yellow, green, blue, grey, black
};

enum class WidgetType : uint8_t { Dial, Ring, Bar, Number, Label, Light, Rim, Shape, Path, Image };
enum class Font : uint8_t { Sans, Condensed, Digital, Mono, Display, Carter, Racing, Trade };
// Marker: arrow at radius r pointing inward; MarkerOut: arrow inside r pointing outward.
enum class NeedleStyle : uint8_t { Line, Tapered, TaperedCap, None, Marker, MarkerOut };
enum class BarStyle : uint8_t { Horizontal, Vertical };
enum class Align : uint8_t { Left, Center, Right };
enum class UnitPos : uint8_t { Right, Below, Left, Above };
enum class ShapeKind : uint8_t { Rect, Ellipse, Line, Triangle, Polygon };
enum class Hold : uint8_t { None, Max, Min };
enum class LightShape : uint8_t { Dot, Ring, Square, Text };
enum class TickShape : uint8_t { Line, Triangle };
enum class ArcSide : uint8_t { Auto, Inside, Outside };  // which way arched text faces
enum class NumberFormat : uint8_t { Plain, Gear };  // gear: 0 = N, -1 = R, -2 = P
enum class RimMode : uint8_t { Flash, Fill };
// How an image fills its box: whole and in proportion, filling the box in
// proportion (edges cut off), or stretched to the box.
enum class ImageFit : uint8_t { Contain, Cover, Stretch };

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
    int16_t unitDx = 0;     // nudge of the unit text from its position, px
    int16_t unitDy = 0;
    Align align = Align::Center;
    Hold hold = Hold::None;
    std::string text;
    int16_t rotate = 0;     // label, number, light, bar: degrees clockwise about x, y
    // label along a circle centred on x, y: radius (0 = straight), where the
    // text's middle sits, and whether the letters face in or out.
    int16_t arc = 0;
    int16_t arcAngle = 270;
    ArcSide arcSide = ArcSide::Auto;

    BarStyle barStyle = BarStyle::Horizontal;
    uint8_t cornerRadius = 4;

    LightShape shape = LightShape::Dot;
    Color offColor = 0x3A3A3C;

    // shape and path: drawn as given, or, with a channel, lit while its condition holds
    ShapeKind shapeKind = ShapeKind::Rect;
    bool filled = true;
    Color strokeColor = 0xFFFFFF;
    uint8_t strokeWidth = 0;
    uint8_t opacity = 100;        // percent
    bool hideWhenOff = false;     // draw nothing while the condition is not met
    std::vector<int16_t> points;  // polygon: x, y pairs relative to x, y
    std::string path;             // path: SVG path data
    int16_t box = 24;             // path: the size of the path's own coordinate box

    // image: drawn in the w x h box (with opacity and rotate), or, with a
    // channel, only while its "on" condition holds
    std::string image;            // name in the hub's image library
    ImageFit fit = ImageFit::Contain;

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
    // An image over the background colour, filling the round face, behind
    // the widgets. Below 100% opacity the colour shows through, which
    // darkens a busy picture behind the numbers.
    std::string bgImage;
    uint8_t bgImageOpacity = 100;
    std::vector<Widget> widgets;
};

// The hub adds "images": {"logo": "1A2B3C4D", ...} to a config on its way to
// a gauge: the CRC32 of each image the config uses, which is what the gauge
// stores it under. A config in the library never has one.
struct ImageRef {
    std::string name;
    uint32_t crc = 0;
};

struct Config {
    std::string name;
    Theme theme;
    std::vector<Face> faces;
    bool hasImageMap = false;
    std::vector<ImageRef> images;
};

struct ParseResult {
    bool ok = false;
    std::string error;  // where and why, e.g. "faces[0].widgets[2]: unknown channel 'boost'"
};

// Parses and validates a configuration. On failure `out` is left in an
// unspecified state and must not be used.
ParseResult parseConfig(const uint8_t* json, size_t size, Config& out);

// Parses the hub's theme colours: a list of 8 to 32 {"value": "#RRGGBB", "name": "..."},
// bare or as {"colours": [...]}. `out` is only changed on success.
ParseResult parseThemeColours(const uint8_t* json, size_t size, ThemeColours& out);
// "#RRGGBB" into `out`, which must hold 8 characters.
void formatColor(Color c, char out[8]);

// 1 to kMaxImageName letters, digits, - and _.
bool validImageName(const char* name);
// The images a config uses (face backgrounds and image widgets), each name
// once, in the order they first appear.
std::vector<std::string> imagesUsed(const Config& config);
// The CRC32 the hub gave an image in the config's map; null if it has none.
const ImageRef* findImage(const Config& config, const std::string& name);
// The images a config's JSON names, read without checking the rest: for the
// hub, which has already validated what it stores.
std::vector<std::string> imagesNamedIn(const uint8_t* json, size_t size);
// The config as it goes to a gauge: the same bytes with "images": {name: CRC}
// added straight after the opening brace. With no refs it is unchanged.
void addImageMap(const uint8_t* json, size_t size, const std::vector<ImageRef>& refs,
                 std::vector<uint8_t>& out);

const char* widgetTypeName(WidgetType type);
const char* fontName(Font font);

}  // namespace cfg
}  // namespace hg
