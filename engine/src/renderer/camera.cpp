#include <engine/renderer/camera.h>

namespace qp {

Vec3 camera_forward(const Camera& cam) {
    f32 yaw_rad   = to_radians(cam.yaw);
    f32 pitch_rad = to_radians(cam.pitch);
    return vec3_normalize({
        cosf(yaw_rad) * cosf(pitch_rad),
        sinf(pitch_rad),
        sinf(yaw_rad) * cosf(pitch_rad)
    });
}

Vec3 camera_right(const Camera& cam) {
    Vec3 fwd = camera_forward(cam);
    return vec3_normalize(vec3_cross(fwd, {0.0f, 1.0f, 0.0f}));
}

Vec3 camera_up(const Camera& cam) {
    Vec3 fwd = camera_forward(cam);
    Vec3 right = camera_right(cam);
    return vec3_cross(right, fwd);
}

Mat4 camera_view_matrix(const Camera& cam) {
    Vec3 fwd = camera_forward(cam);
    Vec3 target = cam.position + fwd;
    return mat4_lookat(cam.position, target, {0.0f, 1.0f, 0.0f});
}

Mat4 camera_projection_matrix(const Camera& cam, f32 aspect) {
    return mat4_perspective(to_radians(cam.fov), aspect, cam.near_plane, cam.far_plane);
}

void camera_process_mouse(Camera& cam, f32 dx, f32 dy) {
    cam.yaw   += dx * cam.sensitivity;
    cam.pitch -= dy * cam.sensitivity;  // inverted Y
    cam.pitch  = clampf(cam.pitch, -89.0f, 89.0f);
}

void camera_process_movement(Camera& cam, f32 forward, f32 right, f32 up, f32 dt) {
    Vec3 fwd = camera_forward(cam);
    // FPS: project forward onto XZ plane for horizontal movement
    Vec3 fwd_flat = vec3_normalize({fwd.x, 0.0f, fwd.z});
    Vec3 right_dir = camera_right(cam);

    Vec3 move = fwd_flat * forward + right_dir * right + vec3(0.0f, 1.0f, 0.0f) * up;

    if (vec3_length_sq(move) > 0.001f) {
        move = vec3_normalize(move);
    }

    cam.position += move * (cam.speed * dt);
}

} // namespace qp
