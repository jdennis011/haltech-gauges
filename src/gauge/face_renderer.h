#pragma once

#include <lvgl.h>
#include <stddef.h>
#include <stdint.h>

#include "face_model.h"
#include "proto.h"

// Draws one face of a config with LVGL. Every widget is painted by one
// full-screen object's draw callback with LVGL's draw primitives, in the
// config's order, the same way the hub page's canvas preview draws it (see
// drawDial() and friends in web/index.html); text that has to be rotated or
// laid along an arc is an LVGL label on top. Values are polled by update(),
// which invalidates only the widgets whose look changed.
//
// Configs are laid out on a 466 px canvas; everything is scaled to the panel.
namespace face_renderer {

// Replaces whatever is built with `faceIndex` of `config`, on `parent` (the
// screen). The config must stay valid until the next build() or destroy().
void build(lv_obj_t* parent, const hg::cfg::Config& config, size_t faceIndex);
void destroy();

// Polls the channel store, applies flashing, invalidates changes. Call from
// the UI loop at about 30 Hz.
void update(uint32_t nowMs);

// The key-on intro: while set, every widget with a channel shows a made-up
// reading driven by `phase` (0 to 1) instead of its channel: needles up to
// full scale and back, bars fill and empty, digits count, lights on. The
// chase style starts each widget a little after the one before it.
void setIntro(hg::proto::IntroStyle style, float phase);
void endIntro();

// True while the sweep is shown because no live data has ever arrived.
bool demo();
// True once live data has arrived and then stopped.
bool stale();

}  // namespace face_renderer
