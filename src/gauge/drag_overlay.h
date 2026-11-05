#pragma once

#include <stddef.h>

#include "face_model.h"

// The drag timer every gauge has whatever its config (docs/motion.md, "The
// overlay"): a pull up from the bottom edge shows it over the face, one
// built-in face per run type with swipes left and right between them, and
// another pull up puts the gauge's own face back. Nothing of it runs until it
// is shown: its config is parsed on the first pull and its timer starts then.
namespace drag_overlay {

bool active();

// Shows it, or hides it; the caller rebuilds the screen afterwards.
void toggle();

// The config drawn while active (null if its JSON did not parse) and the face
// of it for the chosen run type, which is kept in flash across restarts.
const hg::cfg::Config* config();
size_t face();
void step(int delta);

}  // namespace drag_overlay
