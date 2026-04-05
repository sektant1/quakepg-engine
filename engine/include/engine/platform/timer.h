#pragma once

#include <engine/core/types.h>

namespace qp {

void timer_init();
void timer_update(); // call once per frame
f64  timer_delta();  // seconds since last frame
f64  timer_elapsed(); // seconds since init
u32  timer_fps();

} // namespace qp
