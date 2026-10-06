#include <string.h>
#include <unity.h>

#include <string>

#include "face_model.h"
#include "haltech.h"
#include "image_format.h"

using namespace hg;
using namespace hg::cfg;

void setUp() {}
void tearDown() {}

namespace {

ParseResult parse(const std::string& json, Config& out) {
    return parseConfig((const uint8_t*)json.data(), json.size(), out);
}

// Wraps one widget in a minimal valid config.
std::string withWidget(const std::string& widget) {
    return R"({"schema":1,"faces":[{"widgets":[)" + widget + "]}]}";
}

void assertRejected(const std::string& json, const char* expectInError) {
    Config c;
    ParseResult r = parse(json, c);
    TEST_ASSERT_FALSE_MESSAGE(r.ok, json.c_str());
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(r.error.c_str(), expectInError), r.error.c_str());
}

const char* kFull = R"json({
  "schema": 1,
  "name": "Street",
  "theme": "amber",
  "faces": [
    {
      "name": "Boost",
      "widgets": [
        { "type": "dial", "channel": "manifold_pressure", "unit": "psi_gauge",
          "min": -15, "max": 30, "major": 9, "minor": 5, "needle": "tapered_cap",
          "needleColor": "#FF0000", "labelFont": "condensed",
          "zones": [ { "from": 22, "to": 30, "color": "alert" } ] },
        { "type": "number", "channel": "manifold_pressure", "unit": "psi_gauge",
          "y": 330, "font": "digital", "size": 64, "decimals": 1,
          "warn": { "above": 25, "flash": true } },
        { "type": "label", "text": "BOOST", "y": 140, "size": 28, "color": "dim" }
      ]
    },
    {
      "name": "Temps",
      "bg": "#101010",
      "widgets": [
        { "type": "ring", "channel": "coolant_temp", "min": 40, "max": 130,
          "thickness": 24, "segments": 20, "rounded": false },
        { "type": "bar", "channel": "oil_pressure", "unit": "psi", "min": 0, "max": 100,
          "style": "vertical", "x": 120 },
        { "type": "light", "channel": "check_engine_light", "x": 233, "y": 60, "text": "CEL" },
        { "type": "light", "channel": "oil_pressure", "on": { "below": 100 }, "flash": true },
        { "type": "rim", "channel": "rpm", "mode": "fill", "from": 5000, "to": 7000 },
        { "type": "rim", "channel": "rpm", "from": 7200 }
      ]
    }
  ]
})json";

}  // namespace

void test_full_config_parses() {
    Config c;
    ParseResult r = parse(kFull, c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    TEST_ASSERT_EQUAL_STRING("Street", c.name.c_str());
    TEST_ASSERT_EQUAL_HEX32(0xFFB000, c.theme.fg);
    TEST_ASSERT_EQUAL(2, c.faces.size());

    const Face& boost = c.faces[0];
    TEST_ASSERT_EQUAL_STRING("Boost", boost.name.c_str());
    TEST_ASSERT_EQUAL_HEX32(0x000000, boost.bg);
    TEST_ASSERT_EQUAL(3, boost.widgets.size());

    const Widget& dial = boost.widgets[0];
    TEST_ASSERT_EQUAL(int(WidgetType::Dial), int(dial.type));
    TEST_ASSERT_EQUAL(CH_manifold_pressure, dial.channel);
    TEST_ASSERT_EQUAL_STRING("psi", dial.unit->symbol);
    TEST_ASSERT_EQUAL_FLOAT(-15, dial.min);
    TEST_ASSERT_EQUAL_FLOAT(30, dial.max);
    TEST_ASSERT_EQUAL(9, dial.major);
    TEST_ASSERT_EQUAL(int(NeedleStyle::TaperedCap), int(dial.needle));
    TEST_ASSERT_EQUAL_HEX32(0xFF0000, dial.needleColor);
    TEST_ASSERT_EQUAL(int(Font::Condensed), int(dial.labelFont));
    TEST_ASSERT_EQUAL(233, dial.x);  // defaults to the centre
    TEST_ASSERT_EQUAL(220, dial.r);
    TEST_ASSERT_EQUAL(196, dial.needleLen);
    TEST_ASSERT_EQUAL_HEX32(0xFFB000, dial.color);  // theme foreground
    TEST_ASSERT_EQUAL(1, dial.zones.size());
    TEST_ASSERT_EQUAL_HEX32(0xFF2D1A, dial.zones[0].color);  // "alert" role in the amber theme

    const Widget& number = boost.widgets[1];
    TEST_ASSERT_EQUAL(int(Font::Digital), int(number.font));
    TEST_ASSERT_EQUAL(64, number.size);
    TEST_ASSERT_EQUAL(1, number.decimals);
    TEST_ASSERT_EQUAL(21, number.unitSize);
    TEST_ASSERT_TRUE(number.warn.enabled);
    TEST_ASSERT_TRUE(number.warn.above);
    TEST_ASSERT_EQUAL_FLOAT(25, number.warn.threshold);
    TEST_ASSERT_TRUE(number.warn.flash);

    const Widget& label = boost.widgets[2];
    TEST_ASSERT_EQUAL(-1, label.channel);
    TEST_ASSERT_EQUAL_STRING("BOOST", label.text.c_str());
    TEST_ASSERT_EQUAL_HEX32(0x3D2A00, label.color);

    const Face& temps = c.faces[1];
    TEST_ASSERT_EQUAL_HEX32(0x101010, temps.bg);
    TEST_ASSERT_EQUAL(6, temps.widgets.size());
    TEST_ASSERT_EQUAL(20, temps.widgets[0].segments);
    TEST_ASSERT_FALSE(temps.widgets[0].rounded);
    TEST_ASSERT_EQUAL(int(BarStyle::Vertical), int(temps.widgets[1].barStyle));
    TEST_ASSERT_EQUAL(28, temps.widgets[1].w);
    TEST_ASSERT_EQUAL(260, temps.widgets[1].h);

    const Widget& cel = temps.widgets[2];
    TEST_ASSERT_TRUE(cel.warn.above);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, cel.warn.threshold);
    TEST_ASSERT_EQUAL_STRING("CEL", cel.text.c_str());

    const Widget& lowOil = temps.widgets[3];
    TEST_ASSERT_FALSE(lowOil.warn.above);
    TEST_ASSERT_EQUAL_FLOAT(100, lowOil.warn.threshold);
    TEST_ASSERT_TRUE(lowOil.warn.flash);

    TEST_ASSERT_EQUAL(int(RimMode::Fill), int(temps.widgets[4].rimMode));
    TEST_ASSERT_EQUAL(int(RimMode::Flash), int(temps.widgets[5].rimMode));
    TEST_ASSERT_EQUAL_FLOAT(7200, temps.widgets[5].from);
}

void test_custom_theme_object() {
    Config c;
    ParseResult r = parse(
        R"({"schema":1,"theme":{"fg":"#0af","accent":"#123456"},
            "faces":[{"widgets":[{"type":"label","text":"x","color":"accent"}]}]})",
        c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    TEST_ASSERT_EQUAL_HEX32(0x00AAFF, c.theme.fg);
    TEST_ASSERT_EQUAL_HEX32(0x123456, c.faces[0].widgets[0].color);
    TEST_ASSERT_EQUAL_HEX32(0x3A3A3C, c.theme.dim);  // unspecified roles keep their defaults
}

void test_unknown_keys_are_ignored() {
    Config c;
    ParseResult r = parse(withWidget(R"({"type":"label","text":"x","futureOption":42})"), c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
}

void test_rejects_broken_documents() {
    assertRejected("", "not valid JSON");
    assertRejected("{\"schema\":1,\"faces\":[", "not valid JSON");
    assertRejected("[1,2,3]", "must be a JSON object");
    assertRejected(R"({"faces":[]})", "'schema' is required");
    assertRejected(R"({"schema":2,"faces":[]})", "newer than this firmware");
    assertRejected(R"({"schema":1})", "at least one face");
    assertRejected(R"({"schema":1,"faces":[]})", "at least one face");
    assertRejected(R"({"schema":1,"faces":[{"widgets":[]}]})", "at least one widget");
    assertRejected(R"({"schema":1,"theme":"purple","faces":[{"widgets":[]}]})", "unknown theme");
    assertRejected(std::string(kMaxConfigBytes + 1, ' '), "32 KB");
}

void test_rejects_bad_widgets() {
    assertRejected(withWidget(R"({"channel":"rpm"})"), "'type' is required");
    assertRejected(withWidget(R"({"type":"sparkline","channel":"rpm"})"), "unknown widget type");
    assertRejected(withWidget(R"({"type":"number"})"), "'channel' is required");
    assertRejected(withWidget(R"({"type":"number","channel":"warp_factor"})"), "unknown channel");
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","unit":"psi"})"), "not available");
    assertRejected(withWidget(R"({"type":"dial","channel":"rpm"})"), "'min' is required");
    assertRejected(withWidget(R"({"type":"dial","channel":"rpm","min":8000,"max":0})"),
                   "'min' must be less than 'max'");
    assertRejected(withWidget(R"({"type":"dial","channel":"rpm","min":0,"max":8000,"r":500})"),
                   "'r' must be between");
    assertRejected(withWidget(R"({"type":"dial","channel":"rpm","min":0,"max":8000,"needle":"spoon"})"),
                   "'needle': unknown value");
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","color":"chartreuse"})"),
                   "not a colour");
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","size":"big"})"),
                   "'size' must be a number");
    assertRejected(withWidget(R"({"type":"label"})"), "'text' is required");
    assertRejected(withWidget(R"({"type":"label","text":"this caption is much too long to fit on a gauge"})"),
                   "longer than");
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","warn":{"above":1,"below":2}})"),
                   "exactly one of");
    assertRejected(withWidget(R"({"type":"rim","channel":"rpm","mode":"fill","from":7000,"to":5000})"),
                   "'from' must be less than 'to'");
    assertRejected(withWidget(R"({"type":"ring","channel":"rpm","min":0,"max":8000,"r":50,"thickness":60})"),
                   "'thickness' must be less than 'r'");
    assertRejected(withWidget(R"({"type":"ring","channel":"rpm","min":0,"max":8000,
                                  "zones":[{"from":1,"to":2},{"from":1,"to":2},{"from":1,"to":2},
                                           {"from":1,"to":2},{"from":1,"to":2},{"from":1,"to":2},
                                           {"from":1,"to":2}]})"),
                   "zones");
}

void test_errors_say_where() {
    Config c;
    ParseResult r = parse(
        R"({"schema":1,"faces":[{"widgets":[{"type":"label","text":"ok"}]},
            {"widgets":[{"type":"label","text":"ok"},{"type":"number","channel":"nope"}]}]})",
        c);
    TEST_ASSERT_FALSE(r.ok);
    TEST_ASSERT_EQUAL_STRING("faces[1].widgets[1]: unknown channel 'nope'", r.error.c_str());
}

void test_limits() {
    std::string widgets;
    for (size_t i = 0; i < kMaxWidgetsPerFace + 1; i++) {
        widgets += (i ? "," : "");
        widgets += R"({"type":"label","text":"x"})";
    }
    assertRejected(withWidget(widgets), "more than 16 widgets");

    std::string faces;
    for (size_t i = 0; i < kMaxFaces + 1; i++) {
        faces += (i ? "," : "");
        faces += R"({"widgets":[{"type":"label","text":"x"}]})";
    }
    assertRejected("{\"schema\":1,\"faces\":[" + faces + "]}", "more than 8 faces");
}

void test_scale_without_needle_and_text_light() {
    Config c;
    const std::string json = withWidget(
        R"({"type":"dial","channel":"coolant_temp","min":20,"max":120,"needle":"none","ticks":false,"arcWidth":3})");
    TEST_ASSERT_TRUE(parse(json, c).ok);
    const Widget& dial = c.faces[0].widgets[0];
    TEST_ASSERT_EQUAL(int(NeedleStyle::None), int(dial.needle));
    TEST_ASSERT_FALSE(dial.ticks);
    TEST_ASSERT_EQUAL(3, dial.arcWidth);

    const std::string light = withWidget(
        R"({"type":"light","channel":"coolant_temp","shape":"text","text":"COOLANT LOW","on":{"below":60}})");
    TEST_ASSERT_TRUE(parse(light, c).ok);
    TEST_ASSERT_EQUAL(int(LightShape::Text), int(c.faces[0].widgets[0].shape));
    TEST_ASSERT_FALSE(c.faces[0].widgets[0].warn.above);
    assertRejected(withWidget(R"({"type":"light","channel":"coolant_temp","shape":"text"})"),
                   "'text' is required for shape 'text'");
}

void test_nfs_options() {
    Config c;
    TEST_ASSERT_TRUE(parse(withWidget(
        R"({"type":"dial","channel":"rpm","min":0,"max":8000,"needle":"none","labels":false,"tickShape":"triangle"})"), c).ok);
    TEST_ASSERT_EQUAL(int(TickShape::Triangle), int(c.faces[0].widgets[0].tickShape));

    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"dial","channel":"vehicle_speed","min":0,"max":200,"needle":"marker"})"), c).ok);
    TEST_ASSERT_EQUAL(int(NeedleStyle::Marker), int(c.faces[0].widgets[0].needle));
    TEST_ASSERT_EQUAL(28, c.faces[0].widgets[0].needleLen);
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"dial","channel":"vehicle_speed","min":0,"max":200,"needle":"marker_out","r":180})"), c).ok);
    TEST_ASSERT_EQUAL(int(NeedleStyle::MarkerOut), int(c.faces[0].widgets[0].needle));

    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"number","channel":"gear","format":"gear","showUnit":false})"), c).ok);
    TEST_ASSERT_EQUAL(int(NumberFormat::Gear), int(c.faces[0].widgets[0].format));

    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"number","channel":"vehicle_speed","unit":"mph","pad":3})"), c).ok);
    TEST_ASSERT_EQUAL(3, c.faces[0].widgets[0].pad);
    assertRejected(withWidget(R"({"type":"number","channel":"vehicle_speed","pad":7})"), "'pad' must be between 0 and 6");

    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"label","text":"THROTTLE","rotate":60})"), c).ok);
    TEST_ASSERT_EQUAL(60, c.faces[0].widgets[0].rotate);
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"number","channel":"rpm","rotate":-90})"), c).ok);
    TEST_ASSERT_EQUAL(-90, c.faces[0].widgets[0].rotate);
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"bar","channel":"rpm","min":0,"max":8000,"rotate":45})"), c).ok);
    TEST_ASSERT_EQUAL(45, c.faces[0].widgets[0].rotate);
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","rotate":200})"), "'rotate' must be between -180 and 180");
}

void test_arc_label() {
    Config c;
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"label","text":"WATER TEMP","arc":200,"arcAngle":-90,"arcSide":"outside"})"), c).ok);
    const Widget& w = c.faces[0].widgets[0];
    TEST_ASSERT_EQUAL(200, w.arc);
    TEST_ASSERT_EQUAL(270, w.arcAngle);
    TEST_ASSERT_EQUAL(int(ArcSide::Outside), int(w.arcSide));
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"label","text":"x"})"), c).ok);
    TEST_ASSERT_EQUAL(0, c.faces[0].widgets[0].arc);
    assertRejected(withWidget(R"({"type":"label","text":"x","arc":300})"), "'arc' must be between 0 and 233");
    assertRejected(withWidget(R"({"type":"label","text":"x","arc":100,"arcSide":"sideways"})"), "'arcSide': unknown value");
}

void test_shapes_paths_and_unit_position() {
    Config c;
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"shape","shape":"ellipse","w":80,"h":40,"filled":false,"strokeWidth":4,"rotate":30})"), c).ok);
    TEST_ASSERT_EQUAL(int(ShapeKind::Ellipse), int(c.faces[0].widgets[0].shapeKind));
    TEST_ASSERT_EQUAL(-1, c.faces[0].widgets[0].channel);
    TEST_ASSERT_FALSE(c.faces[0].widgets[0].warn.enabled);

    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"shape","shape":"polygon","points":[0,-20,20,20,-20,20]})"), c).ok);
    TEST_ASSERT_EQUAL(6, c.faces[0].widgets[0].points.size());
    assertRejected(withWidget(R"({"type":"shape","shape":"polygon","points":[0,0,10,10]})"), "'points' must list 3 to 16");
    assertRejected(withWidget(R"({"type":"shape","shape":"blob"})"), "'shape': unknown value");

    // A path tied to a channel lights on its condition, like a light.
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"path","d":"M12 2 L1 21 H23 Z","size":64,"channel":"check_engine_light","hideWhenOff":true})"), c).ok);
    TEST_ASSERT_TRUE(c.faces[0].widgets[0].warn.enabled);
    TEST_ASSERT_TRUE(c.faces[0].widgets[0].hideWhenOff);
    TEST_ASSERT_EQUAL(64, c.faces[0].widgets[0].size);
    assertRejected(withWidget(R"({"type":"path"})"), "'d' is required");
    assertRejected(withWidget(R"({"type":"path","d":"M0 0 <script>"})"), "not SVG path data");

    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"number","channel":"rpm","unitPos":"above","unitDx":-12,"unitDy":4})"), c).ok);
    TEST_ASSERT_EQUAL(int(UnitPos::Above), int(c.faces[0].widgets[0].unitPos));
    TEST_ASSERT_EQUAL(-12, c.faces[0].widgets[0].unitDx);
}

void test_theme_colours() {
    Config c;
    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"number","channel":"rpm","color":"themecolour2"})"), c).ok);
    const Color col = c.faces[0].widgets[0].color;
    TEST_ASSERT_TRUE(isThemeColourRef(col));
    TEST_ASSERT_EQUAL(1, themeColourIndex(col));

    // Drawn in whatever the hub has; plain colours pass through.
    ThemeColours tc;
    TEST_ASSERT_EQUAL_HEX32(0xFF8C00, resolveColor(col, tc.value));
    tc.value[1] = 0x123456;
    TEST_ASSERT_EQUAL_HEX32(0x123456, resolveColor(col, tc.value));
    TEST_ASSERT_EQUAL_HEX32(0xFF0000, resolveColor(0xFF0000, tc.value));

    // In a config's own theme, every widget using that role follows the hub colour. US spelling too.
    TEST_ASSERT_TRUE(parse(R"({"schema":1,"theme":{"fg":"themecolor8"},"faces":[{"widgets":[{"type":"number","channel":"rpm"}]}]})", c).ok);
    TEST_ASSERT_EQUAL_HEX32(themeColourRef(7), c.faces[0].widgets[0].color);

    TEST_ASSERT_TRUE(parse(withWidget(R"({"type":"number","channel":"rpm","color":"themecolour32"})"), c).ok);
    TEST_ASSERT_EQUAL(31, themeColourIndex(c.faces[0].widgets[0].color));
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","color":"themecolour33"})"), "themecolour1 to themecolour32");
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","color":"themecolour0"})"), "themecolour1 to themecolour32");
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","color":"themecolour05"})"), "themecolour1 to themecolour32");
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","color":"themecolours"})"), "themecolour1 to themecolour32");
    // A slot the hub has not filled is drawn white.
    TEST_ASSERT_EQUAL_HEX32(kUnsetThemeColour, resolveColor(themeColourRef(20), ThemeColours().value));
    assertRejected(withWidget(R"({"type":"number","channel":"rpm","color":"purple"})"), "not a colour");

    // The hub's list.
    std::string list = "[";
    for (int i = 0; i < 8; i++) list += std::string(i ? "," : "") + R"({"value":"#F80","name":"Orange"})";
    list += "]";
    ThemeColours parsed;
    TEST_ASSERT_TRUE(parseThemeColours(reinterpret_cast<const uint8_t*>(list.data()), list.size(), parsed).ok);
    TEST_ASSERT_EQUAL_HEX32(0xFF8800, parsed.value[7]);
    TEST_ASSERT_EQUAL_STRING("Orange", parsed.name[0].c_str());
    char hex[8];
    formatColor(parsed.value[0], hex);
    TEST_ASSERT_EQUAL_STRING("#FF8800", hex);

    const char* seven = R"([{"value":"#FFF"},{"value":"#FFF"},{"value":"#FFF"},{"value":"#FFF"},{"value":"#FFF"},{"value":"#FFF"},{"value":"#FFF"}])";
    ParseResult r = parseThemeColours(reinterpret_cast<const uint8_t*>(seven), strlen(seven), parsed);
    TEST_ASSERT_FALSE(r.ok);
    TEST_ASSERT_NOT_NULL(strstr(r.error.c_str(), "list of 8 to 32"));

    // Added colours after the first eight; more than 32 is refused.
    std::string nine = list.substr(0, list.size() - 1) + R"(,{"value":"#123456","name":"Extra"}])";
    TEST_ASSERT_TRUE(parseThemeColours(reinterpret_cast<const uint8_t*>(nine.data()), nine.size(), parsed).ok);
    TEST_ASSERT_EQUAL(9, parsed.count);
    TEST_ASSERT_EQUAL_HEX32(0x123456, parsed.value[8]);
    TEST_ASSERT_EQUAL_HEX32(kUnsetThemeColour, parsed.value[9]);
    std::string many = "[";
    for (int i = 0; i < 33; i++) many += std::string(i ? "," : "") + R"({"value":"#F80"})";
    many += "]";
    r = parseThemeColours(reinterpret_cast<const uint8_t*>(many.data()), many.size(), parsed);
    TEST_ASSERT_FALSE(r.ok);
    std::string bad = list;
    bad.replace(bad.find("#F80"), 4, "red!");
    r = parseThemeColours(reinterpret_cast<const uint8_t*>(bad.data()), bad.size(), parsed);
    TEST_ASSERT_NOT_NULL(strstr(r.error.c_str(), "theme colour 1: 'value'"));
    TEST_ASSERT_EQUAL_HEX32(0xFF8800, parsed.value[0]);  // unchanged on failure
}

void test_theme_presets_exist() {
    size_t count = 0;
    const ThemePreset* presets = themePresets(count);
    TEST_ASSERT_GREATER_OR_EQUAL(4, count);
    TEST_ASSERT_EQUAL_STRING("white", presets[0].name);
    for (size_t i = 0; i < count; i++) {
        Config c;
        std::string json = std::string("{\"schema\":1,\"theme\":\"") + presets[i].name +
                           "\",\"faces\":[{\"widgets\":[{\"type\":\"label\",\"text\":\"x\"}]}]}";
        TEST_ASSERT_TRUE(parse(json, c).ok);
        TEST_ASSERT_EQUAL_HEX32(presets[i].theme.fg, c.theme.fg);
    }
}

void test_images_in_a_config() {
    Config c;
    ParseResult r = parse(R"({"schema":1,"faces":[
        {"name":"A","bgImage":"carbon","bgImageOpacity":40,"widgets":[
          {"type":"image","image":"logo","x":233,"y":120,"w":200,"h":90,"fit":"cover","opacity":80,"rotate":-10},
          {"type":"image","image":"oil-can","channel":"oil_pressure_light","flash":true},
          {"type":"image","image":"logo"}]},
        {"name":"B","bgImage":"logo","widgets":[{"type":"label","text":"x"}]}]})", c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    const Face& a = c.faces[0];
    TEST_ASSERT_EQUAL_STRING("carbon", a.bgImage.c_str());
    TEST_ASSERT_EQUAL(40, a.bgImageOpacity);
    TEST_ASSERT_EQUAL(100, c.faces[1].bgImageOpacity);

    const Widget& logo = a.widgets[0];
    TEST_ASSERT_EQUAL(int(WidgetType::Image), int(logo.type));
    TEST_ASSERT_EQUAL_STRING("logo", logo.image.c_str());
    TEST_ASSERT_EQUAL(200, logo.w);
    TEST_ASSERT_EQUAL(90, logo.h);
    TEST_ASSERT_EQUAL(int(ImageFit::Cover), int(logo.fit));
    TEST_ASSERT_EQUAL(80, logo.opacity);
    TEST_ASSERT_EQUAL(-10, logo.rotate);
    TEST_ASSERT_EQUAL(-1, logo.channel);
    TEST_ASSERT_FALSE(logo.warn.enabled);

    // With a channel, an image shows only while its condition holds.
    const Widget& can = a.widgets[1];
    TEST_ASSERT_EQUAL(160, can.w);
    TEST_ASSERT_EQUAL(int(ImageFit::Contain), int(can.fit));
    TEST_ASSERT_TRUE(can.channel >= 0);
    TEST_ASSERT_TRUE(can.hideWhenOff);
    TEST_ASSERT_TRUE(can.warn.enabled);
    TEST_ASSERT_TRUE(can.warn.flash);

    // Each image once, in the order it first appears.
    const std::vector<std::string> used = imagesUsed(c);
    TEST_ASSERT_EQUAL(3, used.size());
    TEST_ASSERT_EQUAL_STRING("carbon", used[0].c_str());
    TEST_ASSERT_EQUAL_STRING("logo", used[1].c_str());
    TEST_ASSERT_EQUAL_STRING("oil-can", used[2].c_str());
    TEST_ASSERT_FALSE(c.hasImageMap);
    TEST_ASSERT_NULL(findImage(c, "logo"));

    assertRejected(withWidget(R"({"type":"image"})"), "'image' is required");
    assertRejected(withWidget(R"({"type":"image","image":"my logo"})"), "must be an image name");
    assertRejected(withWidget(R"({"type":"image","image":"logo","fit":"tile"})"), "unknown value 'tile'");
    assertRejected(withWidget(R"({"type":"image","image":"logo","w":0})"), "'w' must be between 1 and 466");
}

void test_image_map_from_the_hub() {
    Config c;
    ParseResult r = parse(R"({"images":{"logo":"1A2B3C4D","carbon":"00ff00aa"},"schema":1,
        "faces":[{"bgImage":"carbon","widgets":[{"type":"image","image":"logo"}]}]})", c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    TEST_ASSERT_TRUE(c.hasImageMap);
    TEST_ASSERT_EQUAL(2, c.images.size());
    TEST_ASSERT_EQUAL_HEX32(0x1A2B3C4D, findImage(c, "logo")->crc);
    TEST_ASSERT_EQUAL_HEX32(0x00FF00AA, findImage(c, "carbon")->crc);
    TEST_ASSERT_NULL(findImage(c, "other"));

    assertRejected(R"({"images":{"logo":"1A2B3C4"},"schema":1,"faces":[{"widgets":[{"type":"label","text":"x"}]}]})",
                   "8 hex digits");
    assertRejected(R"({"images":{"logo":"1A2B3C4G"},"schema":1,"faces":[{"widgets":[{"type":"label","text":"x"}]}]})",
                   "8 hex digits");
    assertRejected(R"({"images":["logo"],"schema":1,"faces":[{"widgets":[{"type":"label","text":"x"}]}]})",
                   "'images' must map");
}

void test_image_limit_per_config() {
    // Sixteen images on one face and a seventeenth behind another face is too many.
    std::string json = R"({"schema":1,"faces":[{"widgets":[)";
    for (int i = 0; i < 16; i++) json += std::string(i ? "," : "") + R"({"type":"image","image":"img)" + std::to_string(i) + "\"}";
    json += R"(]},{"bgImage":"one-more","widgets":[{"type":"label","text":"x"}]}]})";
    assertRejected(json, "more than 16 different images");

    // Sixteen is fine, and the same image used twice counts once.
    std::string ok = R"({"schema":1,"faces":[{"bgImage":"img0","widgets":[)";
    for (int i = 0; i < 16; i++) ok += std::string(i ? "," : "") + R"({"type":"image","image":"img)" + std::to_string(i) + "\"}";
    ok += "]}]}";
    Config c;
    ParseResult r = parse(ok, c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    TEST_ASSERT_EQUAL(16, imagesUsed(c).size());
}

namespace {

// A JPEG cut down to its markers: start, an APP0 segment, a frame header.
std::vector<uint8_t> jpegHeader(uint8_t sof, uint16_t w, uint16_t h, uint8_t components = 3) {
    std::vector<uint8_t> j = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F', 0, 1, 1, 0, 0, 1, 0, 1, 0, 0};
    const uint8_t frame[] = {0xFF, sof, 0x00, 0x11, 8, uint8_t(h >> 8), uint8_t(h), uint8_t(w >> 8), uint8_t(w), components};
    j.insert(j.end(), frame, frame + sizeof(frame));
    j.resize(j.size() + 9, 0);
    j.push_back(0xFF);
    j.push_back(0xD9);
    return j;
}

std::vector<uint8_t> pngHeader(uint32_t w, uint32_t h) {
    std::vector<uint8_t> p = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n', 0, 0, 0, 13, 'I', 'H', 'D', 'R'};
    for (uint32_t v : {w, h}) {
        for (int s = 24; s >= 0; s -= 8) p.push_back(uint8_t(v >> s));
    }
    const uint8_t rest[] = {8, 6, 0, 0, 0, 0, 0, 0, 0};  // depth, RGBA, ..., CRC
    p.insert(p.end(), rest, rest + sizeof(rest));
    return p;
}

void assertProbeFails(const std::vector<uint8_t>& file, const char* expect) {
    ImageInfo info;
    std::string error;
    TEST_ASSERT_FALSE(probeImage(file.data(), file.size(), info, error));
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(error.c_str(), expect), error.c_str());
}

}  // namespace

void test_image_headers() {
    ImageInfo info;
    std::string error;
    const std::vector<uint8_t> jpeg = jpegHeader(0xC0, 466, 300);
    TEST_ASSERT_TRUE_MESSAGE(probeImage(jpeg.data(), jpeg.size(), info, error), error.c_str());
    TEST_ASSERT_EQUAL(int(ImageType::Jpeg), int(info.type));
    TEST_ASSERT_EQUAL(466, info.width);
    TEST_ASSERT_EQUAL(300, info.height);
    TEST_ASSERT_EQUAL_STRING("jpeg", imageTypeName(info.type));

    const std::vector<uint8_t> grey = jpegHeader(0xC0, 64, 64, 1);
    TEST_ASSERT_TRUE(probeImage(grey.data(), grey.size(), info, error));

    const std::vector<uint8_t> png = pngHeader(120, 80);
    TEST_ASSERT_TRUE_MESSAGE(probeImage(png.data(), png.size(), info, error), error.c_str());
    TEST_ASSERT_EQUAL(int(ImageType::Png), int(info.type));
    TEST_ASSERT_EQUAL(120, info.width);
    TEST_ASSERT_EQUAL(80, info.height);

    assertProbeFails(jpegHeader(0xC2, 100, 100), "progressive JPEG");
    assertProbeFails(jpegHeader(0xC3, 100, 100), "lossless or arithmetic");
    assertProbeFails(jpegHeader(0xC0, 100, 100, 4), "CMYK");
    assertProbeFails(jpegHeader(0xC0, 800, 600), "800 x 600");
    assertProbeFails(pngHeader(467, 10), "at most 466 x 466");
    assertProbeFails(pngHeader(0, 10), "no pixels");
    std::vector<uint8_t> cut = jpegHeader(0xC0, 100, 100);
    cut.resize(12);
    assertProbeFails(cut, "damaged or cut short");
    assertProbeFails(std::vector<uint8_t>{'G', 'I', 'F', '8', '9', 'a', 0, 0, 0, 0}, "not a JPEG or PNG");
    assertProbeFails(std::vector<uint8_t>{0xFF, 0xD8}, "too short");
}

void test_hub_adds_the_image_map() {
    const std::string json = R"(  {"schema":1,"faces":[{"bgImage":"carbon","widgets":[
        {"type":"image","image":"logo"},{"type":"label","text":"x","image":"not-an-image-widget"},
        {"type":"image","image":"logo"}]}]})";
    const uint8_t* data = (const uint8_t*)json.data();
    const std::vector<std::string> names = imagesNamedIn(data, json.size());
    TEST_ASSERT_EQUAL(2, names.size());
    TEST_ASSERT_EQUAL_STRING("carbon", names[0].c_str());
    TEST_ASSERT_EQUAL_STRING("logo", names[1].c_str());

    // The hub has both: the map goes in after the opening brace, the rest untouched.
    std::vector<uint8_t> out;
    addImageMap(data, json.size(), {{"carbon", 0x0000ABCD}, {"logo", 0xDEADBEEF}}, out);
    const std::string text(out.begin(), out.end());
    TEST_ASSERT_EQUAL_STRING(R"(  {"images":{"carbon":"0000ABCD","logo":"DEADBEEF"},"schema":1,)",
                             text.substr(0, text.find("\"faces\"")).c_str());
    TEST_ASSERT_EQUAL_STRING(json.substr(json.find("\"faces\"")).c_str(), text.substr(text.find("\"faces\"")).c_str());

    // The gauge reads it back.
    Config c;
    ParseResult r = parseConfig(out.data(), out.size(), c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, findImage(c, "logo")->crc);
    TEST_ASSERT_EQUAL_HEX32(0x0000ABCD, findImage(c, "carbon")->crc);

    // No images on the hub: the config goes as it is.
    addImageMap(data, json.size(), {}, out);
    TEST_ASSERT_TRUE(std::string(out.begin(), out.end()) == json);
    TEST_ASSERT_EQUAL(0, imagesNamedIn((const uint8_t*)"not json", 8).size());

    // A config at the size limit still fits with sixteen long names in its map.
    std::vector<ImageRef> refs;
    for (int i = 0; i < 16; i++) refs.push_back({std::string(30, 'a') + std::to_string(10 + i), 0xFFFFFFFF});
    std::string big = R"({"schema":1,"name":"padding","faces":[{"widgets":[{"type":"label","text":"x"}]}],"pad":")";
    big += std::string(kMaxConfigBytes - big.size() - 2, ' ') + "\"}";
    TEST_ASSERT_EQUAL(kMaxConfigBytes, big.size());
    addImageMap((const uint8_t*)big.data(), big.size(), refs, out);
    TEST_ASSERT_TRUE(out.size() <= kMaxConfigBytes + kMaxImageMapBytes);
    r = parseConfig(out.data(), out.size(), c);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, r.error.c_str());
    TEST_ASSERT_EQUAL(16, c.images.size());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_full_config_parses);
    RUN_TEST(test_custom_theme_object);
    RUN_TEST(test_unknown_keys_are_ignored);
    RUN_TEST(test_rejects_broken_documents);
    RUN_TEST(test_rejects_bad_widgets);
    RUN_TEST(test_errors_say_where);
    RUN_TEST(test_limits);
    RUN_TEST(test_scale_without_needle_and_text_light);
    RUN_TEST(test_nfs_options);
    RUN_TEST(test_arc_label);
    RUN_TEST(test_shapes_paths_and_unit_position);
    RUN_TEST(test_theme_colours);
    RUN_TEST(test_theme_presets_exist);
    RUN_TEST(test_images_in_a_config);
    RUN_TEST(test_image_map_from_the_hub);
    RUN_TEST(test_image_limit_per_config);
    RUN_TEST(test_image_headers);
    RUN_TEST(test_hub_adds_the_image_map);
    return UNITY_END();
}
