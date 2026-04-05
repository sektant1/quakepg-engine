#include <engine/platform/timer.h>

#include <GLFW/glfw3.h>

namespace qp {

static f64 s_last_time   = 0.0;
static f64 s_delta       = 0.0;
static f64 s_start_time  = 0.0;
static u32 s_fps         = 0;
static u32 s_frame_count = 0;
static f64 s_fps_timer   = 0.0;

void timer_init() {
    s_start_time = glfwGetTime();
    s_last_time  = s_start_time;
    s_delta      = 0.0;
    s_fps        = 0;
    s_frame_count = 0;
    s_fps_timer  = s_start_time;
}

void timer_update() {
    f64 now  = glfwGetTime();
    s_delta  = now - s_last_time;
    s_last_time = now;

    s_frame_count++;
    if (now - s_fps_timer >= 1.0) {
        s_fps = s_frame_count;
        s_frame_count = 0;
        s_fps_timer = now;
    }
}

f64 timer_delta()   { return s_delta; }
f64 timer_elapsed() { return glfwGetTime() - s_start_time; }
u32 timer_fps()     { return s_fps; }

} // namespace qp
