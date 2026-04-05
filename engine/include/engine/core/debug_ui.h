#pragma once

#include <engine/core/types.h>
#include <engine/core/math.h>
#include <engine/renderer/camera.h>
#include <engine/renderer/renderer.h>

namespace qp {

struct DebugUIState
{
    // Renderer
    Renderer *renderer;
    i32       fb_w, fb_h;
    i32       internal_w, internal_h;
    i32       resolution_preset; // 0=320x240, 1=512x384, 2=640x480, 3=native

    // Camera
    Camera *camera;
    f32     base_speed;
    f32     sprint_mult;

    // PSX
    f32   snap_resolution;
    f32   fog_color[3];
    f32   clear_color[3];
    f32   fog_start;
    f32   fog_end;
    bool  dithering_enabled;
    f32   color_depth;
    f32   tint_color[4];

    // Weapon (optional, nullptr if no weapon)
    Vec3 *weapon_offset;
    Vec3 *weapon_rotation;
    f32  *weapon_scale;
};

// Draw the PSX Debug window. Pass open flag to show/hide.
void debug_ui_draw(DebugUIState &state, bool *open);

} // namespace qp
