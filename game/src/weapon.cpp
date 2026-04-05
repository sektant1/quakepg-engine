#include <game/weapon.h>
#include <engine/core/log.h>

#include <glad/glad.h>
#include <cmath>

namespace qp
{

Weapon weapon_create(const char *model_path)
{
    Weapon w   = {};
    w.model    = model_load(model_path);
    w.offset   = {0.46f, -0.51f, -0.94f};  // right, down, forward (further away)
    w.rotation = {-10.0f, -80.0f, -2.0f};  // tilted sideways, blade pointing up-right
    w.scale    = 0.06f;

    w.anim_time     = 0.0f;
    w.anim_duration = 0.35f;
    w.is_attacking  = false;
    w.bob_time      = 0.0f;

    return w;
}

void weapon_destroy(Weapon &w)
{
    model_destroy(w.model);
}

void weapon_attack(Weapon &w)
{
    if (!w.is_attacking) {
        w.is_attacking = true;
        w.anim_time    = 0.0f;
    }
}

void weapon_update(Weapon &w, f32 dt, f32 move_speed)
{
    // Attack animation
    if (w.is_attacking) {
        w.anim_time += dt;
        if (w.anim_time >= w.anim_duration) {
            w.is_attacking = false;
            w.anim_time    = 0.0f;
        }
    }

    // Idle/walk bob
    if (move_speed > 0.1f) {
        w.bob_time += dt * 8.0f;
    } else {
        w.bob_time += dt * 1.5f;
    }
}

void weapon_draw(const Weapon &w, const Camera &cam, const Shader &shader, f32 aspect)
{
    // Weapon uses its own near/far to avoid clipping
    f32  weapon_fov  = 70.0f;
    Mat4 weapon_proj = mat4_perspective(to_radians(weapon_fov), aspect, 0.01f, 10.0f);

    // Build view-space model matrix
    Vec3 pos = w.offset;
    Vec3 rot = w.rotation;

    // Idle bob
    f32 bob_x = sinf(w.bob_time) * 0.005f;
    f32 bob_y = sinf(w.bob_time * 2.0f) * 0.003f;
    pos.x += bob_x;
    pos.y += bob_y;

    // Attack animation: swing forward and rotate
    if (w.is_attacking) {
        f32 t = w.anim_time / w.anim_duration;
        // Sine curve: quick forward thrust, then return
        f32 swing = sinf(t * 3.14159f);
        pos.z -= swing * 0.2f;  // thrust forward
        pos.y += swing * 0.2f;  // slight lift
        pos.x -= swing * 0.2f;
        rot.x -= swing * 30.0f;  // pitch down (slash motion)
        rot.z += swing * 50.0f;  // pitch down (slash motion)
    }

    Mat4 model_mat = mat4_translate(pos) * mat4_rotate_x(to_radians(rot.x)) * mat4_rotate_y(to_radians(rot.y))
        * mat4_rotate_z(to_radians(rot.z)) * mat4_scale({w.scale, w.scale, w.scale});

    // The weapon is rendered in vi
    // ew space, so the view matrix is identity
    Mat4 view_identity = mat4_identity();

    // Clear depth so weapon always renders on top
    glClear(GL_DEPTH_BUFFER_BIT);

    shader_bind(shader);
    shader_set_mat4(shader, "uProjection", weapon_proj.data);
    shader_set_mat4(shader, "uView", view_identity.data);
    shader_set_mat4(shader, "uModel", model_mat.data);
    shader_set_int(shader, "uUseTexture", 1);
    shader_set_float(shader, "uSnapResolution", 0.0f);  // no jitter on weapon
    shader_set_float(shader, "uFogStart", 100.0f);      // disable fog on weapon
    shader_set_float(shader, "uFogEnd", 200.0f);

    model_draw(w.model);
}

}  // namespace qp
