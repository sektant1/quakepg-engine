#include <engine/platform/input.h>
#include <engine/platform/window.h>
#include <engine/core/assert.h>

#include <GLFW/glfw3.h>
#include <cstring>

namespace qp {

static GLFWwindow* s_window = nullptr;

static bool s_keys_current[(u32)Key::Count]  = {};
static bool s_keys_previous[(u32)Key::Count] = {};

static bool s_mouse_current[(u32)MouseButton::Count]  = {};
static bool s_mouse_previous[(u32)MouseButton::Count] = {};

static f64 s_mouse_x = 0.0, s_mouse_y = 0.0;
static f64 s_mouse_last_x = 0.0, s_mouse_last_y = 0.0;
static f64 s_mouse_dx = 0.0, s_mouse_dy = 0.0;
static f64 s_scroll_dy = 0.0;
static bool s_first_mouse = true;

static int key_to_glfw(Key k) {
    switch (k) {
        case Key::W:         return GLFW_KEY_W;
        case Key::A:         return GLFW_KEY_A;
        case Key::S:         return GLFW_KEY_S;
        case Key::D:         return GLFW_KEY_D;
        case Key::Q:         return GLFW_KEY_Q;
        case Key::E:         return GLFW_KEY_E;
        case Key::R:         return GLFW_KEY_R;
        case Key::F:         return GLFW_KEY_F;
        case Key::Space:     return GLFW_KEY_SPACE;
        case Key::LeftShift: return GLFW_KEY_LEFT_SHIFT;
        case Key::LeftCtrl:  return GLFW_KEY_LEFT_CONTROL;
        case Key::Escape:    return GLFW_KEY_ESCAPE;
        case Key::Tab:       return GLFW_KEY_TAB;
        case Key::Num1:      return GLFW_KEY_1;
        case Key::Num2:      return GLFW_KEY_2;
        case Key::Num3:      return GLFW_KEY_3;
        case Key::Num4:      return GLFW_KEY_4;
        case Key::Num5:      return GLFW_KEY_5;
        case Key::Up:        return GLFW_KEY_UP;
        case Key::Down:      return GLFW_KEY_DOWN;
        case Key::Left:      return GLFW_KEY_LEFT;
        case Key::Right:     return GLFW_KEY_RIGHT;
        default:             return GLFW_KEY_UNKNOWN;
    }
}

static int mouse_to_glfw(MouseButton b) {
    switch (b) {
        case MouseButton::Left:   return GLFW_MOUSE_BUTTON_LEFT;
        case MouseButton::Right:  return GLFW_MOUSE_BUTTON_RIGHT;
        case MouseButton::Middle: return GLFW_MOUSE_BUTTON_MIDDLE;
        default:                  return -1;
    }
}

static void scroll_callback(GLFWwindow*, double, double yoffset) {
    s_scroll_dy = yoffset;
}

void input_init(Window* w) {
    s_window = (GLFWwindow*)window_get_native_handle(w);
    glfwSetScrollCallback(s_window, scroll_callback);
    glfwGetCursorPos(s_window, &s_mouse_x, &s_mouse_y);
    s_mouse_last_x = s_mouse_x;
    s_mouse_last_y = s_mouse_y;
    s_first_mouse = true;
}

void input_update() {
    memcpy(s_keys_previous, s_keys_current, sizeof(s_keys_current));
    memcpy(s_mouse_previous, s_mouse_current, sizeof(s_mouse_current));
    s_scroll_dy = 0.0;
}

void input_poll() {
    glfwPollEvents();

    // update key states
    for (u32 i = 0; i < (u32)Key::Count; i++) {
        int glfw_key = key_to_glfw((Key)i);
        s_keys_current[i] = (glfw_key != GLFW_KEY_UNKNOWN) &&
                            (glfwGetKey(s_window, glfw_key) == GLFW_PRESS);
    }

    // update mouse button states
    for (u32 i = 0; i < (u32)MouseButton::Count; i++) {
        int glfw_btn = mouse_to_glfw((MouseButton)i);
        s_mouse_current[i] = (glfw_btn >= 0) &&
                             (glfwGetMouseButton(s_window, glfw_btn) == GLFW_PRESS);
    }

    // update mouse position
    glfwGetCursorPos(s_window, &s_mouse_x, &s_mouse_y);
    if (s_first_mouse) {
        s_mouse_last_x = s_mouse_x;
        s_mouse_last_y = s_mouse_y;
        s_first_mouse = false;
    }
    s_mouse_dx = s_mouse_x - s_mouse_last_x;
    s_mouse_dy = s_mouse_y - s_mouse_last_y;
    s_mouse_last_x = s_mouse_x;
    s_mouse_last_y = s_mouse_y;
}

bool input_key_down(Key k)     { return s_keys_current[(u32)k]; }
bool input_key_pressed(Key k)  { return s_keys_current[(u32)k] && !s_keys_previous[(u32)k]; }
bool input_key_released(Key k) { return !s_keys_current[(u32)k] && s_keys_previous[(u32)k]; }

bool input_mouse_down(MouseButton b)    { return s_mouse_current[(u32)b]; }
bool input_mouse_pressed(MouseButton b) { return s_mouse_current[(u32)b] && !s_mouse_previous[(u32)b]; }

f32 input_mouse_x()  { return (f32)s_mouse_x; }
f32 input_mouse_y()  { return (f32)s_mouse_y; }
f32 input_mouse_dx() { return (f32)s_mouse_dx; }
f32 input_mouse_dy() { return (f32)s_mouse_dy; }
f32 input_scroll_dy(){ return (f32)s_scroll_dy; }

void input_set_cursor_locked(bool locked) {
    glfwSetInputMode(s_window, GLFW_CURSOR,
                     locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (locked) {
        s_first_mouse = true;
    }
}

} // namespace qp
