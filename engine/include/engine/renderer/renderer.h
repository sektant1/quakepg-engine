#pragma once

#include <engine/core/types.h>

namespace qp {

struct RendererConfig {
    i32 internal_width  = 320;
    i32 internal_height = 240;
};

struct Renderer;

Renderer* renderer_create(const RendererConfig& config);
void      renderer_destroy(Renderer* r);

void renderer_begin_frame(Renderer* r);
void renderer_end_frame(Renderer* r);
void renderer_present(Renderer* r, i32 window_width, i32 window_height);

void renderer_set_clear_color(f32 r, f32 g, f32 b, f32 a);
void renderer_set_viewport(i32 x, i32 y, i32 w, i32 h);

} // namespace qp
