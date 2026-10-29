"""Makes the gauge's boot splash: assets/splash/oagauge.svg (the animation, for
a browser) and src/gauge/splash_art.h (the same lettering as path data, compiled
into the gauge firmware and played by src/gauge/splash.cpp).

The wordmark "oaGauge" and the tag line are turned into outlines from the
bundled fonts, so neither the SVG nor the gauge needs the font at run time.
Edit the constants below, then:

    C:/Users/jdenn/.pioarduino/penv/Scripts/python.exe -I scripts/gen_splash.py

Needs fontTools (pip install fonttools).
"""
import os
import sys

from fontTools.pens.boundsPen import BoundsPen
from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.transformPen import TransformPen
from fontTools.ttLib import TTFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CANVAS = 466  # the design canvas; the gauge scales it to its panel

# The wordmark: each run of letters has its own colour.
WORD = [("o", "#0a84ff"), ("a", "#ff9f0a"), ("Gauge", "#f2f2f7")]
WORD_FONT = "display"      # Russo One
WORD_WIDTH = 350           # px across the canvas
WORD_BASELINE = 252        # y of the baseline

TAG = "open source gauges"
TAG_COLOR = "#8e8e93"
TAG_FONT = "sans"          # Fira Sans
TAG_SIZE = 27              # px
TAG_TRACKING = 1.5         # extra px between letters
TAG_BASELINE = 306

ARC_COLOR = "#0a84ff"
ARC_RADIUS = 222
ARC_WIDTH = 4
ARC_SPAN = 60              # degrees of arc
ARC_PERIOD_MS = 1600       # one turn

# Timing of the entrance, in ms.
LETTER_MS = 350            # each letter fades in and rises over this long
LETTER_STAGGER_MS = 70     # next letter starts this much later
LETTER_RISE = 14           # px
TAG_START_MS = 700
TAG_MS = 400
ARC_FADE_MS = 400


def ntos(v):
    s = "%.1f" % v
    s = s.rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


class Face:
    def __init__(self, key):
        self.font = TTFont(os.path.join(ROOT, "assets", "fonts", key + ".ttf"))
        self.upm = self.font["head"].unitsPerEm
        self.glyphs = self.font.getGlyphSet()
        self.cmap = self.font.getBestCmap()
        self.kern = {}
        if "kern" in self.font:
            for t in self.font["kern"].kernTables:
                self.kern.update(t.kernTable)

    def name(self, ch):
        return self.cmap[ord(ch)]

    def advance(self, ch):
        return self.glyphs[self.name(ch)].width

    def kerning(self, a, b):
        return self.kern.get((self.name(a), self.name(b)), 0)

    def path(self, ch, scale, x, baseline):
        pen = SVGPathPen(self.glyphs, ntos=ntos)
        self.glyphs[self.name(ch)].draw(TransformPen(pen, (scale, 0, 0, -scale, x, baseline)))
        return pen.getCommands()

    def bounds(self, ch, scale, x, baseline):
        pen = BoundsPen(self.glyphs)
        self.glyphs[self.name(ch)].draw(TransformPen(pen, (scale, 0, 0, -scale, x, baseline)))
        return pen.bounds


def union(boxes):
    boxes = [b for b in boxes if b]
    return (min(b[0] for b in boxes), min(b[1] for b in boxes), max(b[2] for b in boxes), max(b[3] for b in boxes))


def layout(face, text, size, tracking=0):
    """[(char, x)] and the total width for `text` at `size` px."""
    scale = size / face.upm
    x = 0.0
    out = []
    for i, ch in enumerate(text):
        if i:
            x += face.kerning(text[i - 1], ch) * scale + tracking
        out.append((ch, x))
        x += face.advance(ch) * scale
    return out, x


def main():
    word_face = Face(WORD_FONT)
    tag_face = Face(TAG_FONT)
    word_text = "".join(t for t, _ in WORD)

    # Size the wordmark to WORD_WIDTH.
    _, w1 = layout(word_face, word_text, 100)
    word_size = 100 * WORD_WIDTH / w1
    letters, width = layout(word_face, word_text, word_size)
    x0 = (CANVAS - width) / 2
    scale = word_size / word_face.upm
    colours = []
    for text, colour in WORD:
        colours += [colour] * len(text)
    word = []  # (char, colour, d, box)
    for (ch, x), colour in zip(letters, colours):
        word.append((ch, colour, word_face.path(ch, scale, x0 + x, WORD_BASELINE),
                     word_face.bounds(ch, scale, x0 + x, WORD_BASELINE)))
    word_box = union(word_face.bounds(ch, scale, x0 + x, WORD_BASELINE) for ch, x in letters)

    tag_letters, tag_width = layout(tag_face, TAG, TAG_SIZE, TAG_TRACKING)
    tx0 = (CANVAS - tag_width) / 2
    tscale = TAG_SIZE / tag_face.upm
    tag = []
    for ch, x in tag_letters:
        if ch == " ":
            continue
        tag.append((ch, tag_face.path(ch, tscale, tx0 + x, TAG_BASELINE)))
    tag_box = union(tag_face.bounds(ch, tscale, tx0 + x, TAG_BASELINE) for ch, x in tag_letters if ch != " ")

    write_svg(word, tag, word_size)
    write_header(word, tag, word_size, word_box, tag_box)


def write_svg(word, tag, word_size):
    L = []
    L.append('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 %d %d" width="%d" height="%d">' % (CANVAS, CANVAS, CANVAS, CANVAS))
    L.append('  <title>oaGauge</title>')
    L.append('  <!-- Made by scripts/gen_splash.py from the bundled fonts; edit that, not this. -->')
    L.append('  <circle cx="%d" cy="%d" r="%d" fill="#000"/>' % (CANVAS / 2, CANVAS / 2, CANVAS / 2))
    spline = 'calcMode="spline" keySplines="0.2 0.8 0.2 1"'
    L.append('  <g id="wordmark" aria-label="oaGauge">')
    for i, (ch, colour, d, _) in enumerate(word):
        start = i * LETTER_STAGGER_MS
        L.append('    <g opacity="0" transform="translate(0 %d)">' % LETTER_RISE)
        L.append('      <animate attributeName="opacity" from="0" to="1" begin="%dms" dur="%dms" fill="freeze"/>' % (start, LETTER_MS))
        L.append('      <animateTransform attributeName="transform" type="translate" from="0 %d" to="0 0" begin="%dms" dur="%dms" fill="freeze" %s/>' % (LETTER_RISE, start, LETTER_MS, spline))
        L.append('      <path fill="%s" d="%s"/>' % (colour, d))
        L.append('    </g>')
    L.append('  </g>')
    L.append('  <g id="tagline" aria-label="%s" opacity="0" fill="%s">' % (TAG, TAG_COLOR))
    L.append('    <animate attributeName="opacity" from="0" to="1" begin="%dms" dur="%dms" fill="freeze"/>' % (TAG_START_MS, TAG_MS))
    for ch, d in tag:
        L.append('    <path d="%s"/>' % d)
    L.append('  </g>')
    # The waiting cue: a short arc going round once a period, for as long as the splash shows.
    c = CANVAS / 2
    r = ARC_RADIUS
    import math
    a = math.radians(ARC_SPAN)
    x1, y1 = c + r, c
    x2, y2 = c + r * math.cos(a), c + r * math.sin(a)
    L.append('  <g id="spinner" opacity="0">')
    L.append('    <animate attributeName="opacity" from="0" to="1" begin="0ms" dur="%dms" fill="freeze"/>' % ARC_FADE_MS)
    L.append('    <path d="M%s %sA%d %d 0 0 1 %s %s" fill="none" stroke="%s" stroke-width="%d" stroke-linecap="round">' % (ntos(x1), ntos(y1), r, r, ntos(x2), ntos(y2), ARC_COLOR, ARC_WIDTH))
    L.append('      <animateTransform attributeName="transform" type="rotate" from="0 %d %d" to="360 %d %d" dur="%dms" repeatCount="indefinite"/>' % (c, c, c, c, ARC_PERIOD_MS))
    L.append('    </path>')
    L.append('  </g>')
    L.append('</svg>')
    out = os.path.join(ROOT, "assets", "splash", "oagauge.svg")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(L) + "\n")
    print("wrote", os.path.relpath(out, ROOT), os.path.getsize(out), "bytes; wordmark %.1f px" % word_size)


def cstr(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def write_header(word, tag, word_size, word_box, tag_box):
    L = []
    L.append("// Generated by scripts/gen_splash.py from assets/splash/oagauge.svg's inputs; do not edit.")
    L.append("// The boot splash's lettering as SVG path data on the %d px canvas, y down." % CANVAS)
    L.append("#pragma once")
    L.append("")
    L.append("#include <stdint.h>")
    L.append("")
    L.append("namespace splash_art {")
    L.append("")
    L.append("struct Glyph {")
    L.append("    uint32_t color;  // 0xRRGGBB")
    L.append("    const char* d;")
    L.append("    float box[4];    // x0 y0 x1 y1 on the canvas")
    L.append("};")
    L.append("")
    L.append("constexpr int kCanvas = %d;" % CANVAS)
    L.append("constexpr int kLetterMs = %d;" % LETTER_MS)
    L.append("constexpr int kLetterStaggerMs = %d;" % LETTER_STAGGER_MS)
    L.append("constexpr float kLetterRise = %d;" % LETTER_RISE)
    L.append("constexpr int kTagStartMs = %d;" % TAG_START_MS)
    L.append("constexpr int kTagMs = %d;" % TAG_MS)
    L.append("constexpr uint32_t kTagColor = 0x%s;" % TAG_COLOR[1:])
    L.append("constexpr uint32_t kArcColor = 0x%s;" % ARC_COLOR[1:])
    L.append("constexpr int kArcRadius = %d;" % ARC_RADIUS)
    L.append("constexpr int kArcWidth = %d;" % ARC_WIDTH)
    L.append("constexpr int kArcSpan = %d;" % ARC_SPAN)
    L.append("constexpr int kArcPeriodMs = %d;" % ARC_PERIOD_MS)
    L.append("constexpr int kArcFadeMs = %d;" % ARC_FADE_MS)
    L.append("")
    L.append("// Bounding boxes, x0 y0 x1 y1, of the lettering at rest.")
    L.append("constexpr float kWordBox[4] = {%s};" % ", ".join(ntos(v) for v in word_box))
    L.append("constexpr float kTagBox[4] = {%s};" % ", ".join(ntos(v) for v in tag_box))
    L.append("")
    L.append("// \"%s\", %.1f px, baseline %d." % ("".join(ch for ch, _, _, _ in word), word_size, WORD_BASELINE))
    L.append("constexpr Glyph kWord[] = {")
    for ch, colour, d, box in word:
        L.append("    {0x%s, %s, {%s}},  // %s" % (colour[1:], cstr(d), ", ".join(ntos(v) for v in box), ch))
    L.append("};")
    L.append("constexpr int kWordCount = %d;" % len(word))
    L.append("")
    L.append("// \"%s\", %d px, baseline %d." % (TAG, TAG_SIZE, TAG_BASELINE))
    L.append("constexpr const char* kTag[] = {")
    for ch, d in tag:
        L.append("    %s,  // %s" % (cstr(d), ch))
    L.append("};")
    L.append("constexpr int kTagCount = %d;" % len(tag))
    L.append("")
    L.append("}  // namespace splash_art")
    out = os.path.join(ROOT, "src", "gauge", "splash_art.h")
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(L) + "\n")
    print("wrote", os.path.relpath(out, ROOT), os.path.getsize(out), "bytes")


if __name__ == "__main__":
    sys.exit(main())
