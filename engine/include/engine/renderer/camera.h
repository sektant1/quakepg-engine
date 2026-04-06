#pragma once

#include <engine/core/types.h>
#include <engine/core/math.h>

namespace qp
{

struct Camera
{
    Vec3 position    = {0.0f, 1.0f, 3.0f};
    f32  yaw         = -90.0f;  // degrees, -90 = looking along -Z
    f32  pitch       = 0.0f;    // degrees
    f32  fov         = 90.0f;   // degrees, wide like Quake
    f32  near_plane  = 0.1f;
    f32  far_plane   = 50.0f;
    f32  sensitivity = 0.15f;
    f32  speed       = 5.0f;
};

Vec3 camera_forward(const Camera &cam);
Vec3 camera_right(const Camera &cam);
Vec3 camera_up(const Camera &cam);

Mat4 camera_view_matrix(const Camera &cam);
Mat4 camera_projection_matrix(const Camera &cam, f32 aspect);

// FPS-style: WASD movement + mouse look
void camera_process_mouse(Camera &cam, f32 dx, f32 dy);
void camera_process_movement(Camera &cam, f32 forward, f32 right, f32 up, f32 dt);

}  // namespace qp
