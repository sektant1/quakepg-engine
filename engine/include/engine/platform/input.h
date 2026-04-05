#pragma once

#include <engine/core/types.h>

namespace qp {

enum class Key : u32 {
    W, A, S, D,
    Q, E, R, F,
    Space, LeftShift, LeftCtrl,
    Escape,
    Tab,
    Num1, Num2, Num3, Num4, Num5,
    Up, Down, Left, Right,
    GraveAccent,
    Count
};

enum class MouseButton : u32 {
    Left, Right, Middle,
    Count
};

struct Window;

void input_init(Window* w);
void input_update(); // call at start of frame (before poll)
void input_poll();   // call after glfwPollEvents

bool input_key_down(Key k);
bool input_key_pressed(Key k);    // true only on first frame
bool input_key_released(Key k);   // true only on release frame

bool input_mouse_down(MouseButton b);
bool input_mouse_pressed(MouseButton b);

f32  input_mouse_x();
f32  input_mouse_y();
f32  input_mouse_dx();
f32  input_mouse_dy();
f32  input_scroll_dy();

void input_set_cursor_locked(bool locked);

} // namespace qp
