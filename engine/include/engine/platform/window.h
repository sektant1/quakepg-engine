#pragma once

#include <engine/core/types.h>

namespace qp {

struct WindowConfig {
    i32 width        = 960;
    i32 height       = 720;
    const char* title = "QuakePG";
    bool vsync       = true;
    bool fullscreen  = false;
};

struct Window;

Window* window_create(const WindowConfig& config);
void    window_destroy(Window* w);
bool    window_should_close(Window* w);
void    window_swap_buffers(Window* w);
void    window_get_size(Window* w, i32* width, i32* height);
void    window_get_framebuffer_size(Window* w, i32* width, i32* height);
void*   window_get_native_handle(Window* w); // returns GLFWwindow*

} // namespace qp
