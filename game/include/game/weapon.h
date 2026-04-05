#pragma once

#include <engine/core/types.h>
#include <engine/core/math.h>
#include <engine/renderer/model.h>
#include <engine/renderer/shader.h>
#include <engine/renderer/camera.h>

namespace qp {

struct Weapon
{
    Model model;

    // Position/rotation offsets in view space (right, down, forward)
    Vec3 offset;
    Vec3 rotation; // degrees (pitch, yaw, roll)
    f32  scale;

    // Animation state
    f32  anim_time;    // current animation timer (0 = idle)
    f32  anim_duration;
    bool is_attacking;

    // Idle bob
    f32 bob_time;
};

// Load weapon model and set default parameters.
Weapon weapon_create(const char *model_path);

// Destroy weapon resources.
void weapon_destroy(Weapon &w);

// Trigger the attack animation.
void weapon_attack(Weapon &w);

// Update animation timers. Call once per frame.
void weapon_update(Weapon &w, f32 dt, f32 move_speed);

// Draw the weapon in first-person view.
// Uses a separate projection to avoid clipping with world geometry.
void weapon_draw(const Weapon &w, const Camera &cam, const Shader &shader, f32 aspect);

} // namespace qp
