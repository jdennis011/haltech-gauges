// Embeds the bundled TTF files from assets/fonts/ into the firmware image.
// Paths are relative to the project root, where the compiler is run from.

    .section .rodata.hg_fonts, "a"

    .macro font name, file
    .global hg_font_\name\()_start
    .global hg_font_\name\()_end
    .balign 4
hg_font_\name\()_start:
    .incbin "\file"
hg_font_\name\()_end:
    .endm

    font sans, "assets/fonts/sans.ttf"
    font condensed, "assets/fonts/condensed.ttf"
    font digital, "assets/fonts/digital.ttf"
    font mono, "assets/fonts/mono.ttf"
    font display, "assets/fonts/display.ttf"
