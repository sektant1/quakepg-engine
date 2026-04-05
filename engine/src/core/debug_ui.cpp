#include <engine/core/debug_ui.h>
#include <engine/platform/timer.h>

#include "imgui.h"

namespace qp
{

void debug_ui_draw(DebugUIState &s, bool *open)
{
    if (!*open) {
        return;
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 200), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Debug", open);

    // -- Performance --
    ImGui::SeparatorText("Performance");
    ImGui::Text("FPS: %u", timer_fps());
    ImGui::Text("Frame: %.2f ms", (float)timer_delta() * 1000.0f);
    ImGui::Text("Pos: %.1f, %.1f, %.1f", s.camera->position.x, s.camera->position.y, s.camera->position.z);

    // -- Resolution --
    ImGui::SeparatorText("Resolution");
    const char *presets[] = {"320x240 (PS1)", "512x384", "640x480", "Native"};
    if (ImGui::Combo("Preset", &s.resolution_preset, presets, 4)) {
        switch (s.resolution_preset) {
            case 0:
                s.internal_w = 320;
                s.internal_h = 240;
                break;
            case 1:
                s.internal_w = 512;
                s.internal_h = 384;
                break;
            case 2:
                s.internal_w = 640;
                s.internal_h = 480;
                break;
            case 3:
                s.internal_w = s.fb_w;
                s.internal_h = s.fb_h;
                break;
        }
        renderer_resize(s.renderer, s.internal_w, s.internal_h);
    }
    ImGui::SliderInt("Width", &s.internal_w, 160, 1920);
    ImGui::SliderInt("Height", &s.internal_h, 120, 1080);
    if (ImGui::Button("Apply Resolution")) {
        renderer_resize(s.renderer, s.internal_w, s.internal_h);
    }
    i32 cur_w, cur_h;
    renderer_get_internal_size(s.renderer, &cur_w, &cur_h);
    ImGui::Text("Current: %dx%d", cur_w, cur_h);

    // -- Camera --
    ImGui::SeparatorText("Camera");
    ImGui::SliderFloat("FOV", &s.camera->fov, 60.0f, 120.0f);
    ImGui::SliderFloat("Speed", &s.base_speed, 1.0f, 20.0f);
    ImGui::SliderFloat("Sprint Multiplier", &s.sprint_mult, 1.0f, 5.0f);
    ImGui::SliderFloat("Sensitivity", &s.camera->sensitivity, 0.05f, 0.5f);

    // -- Vertex Snapping --
    ImGui::SeparatorText("Vertex Snapping");
    ImGui::SliderFloat("Snap Resolution", &s.snap_resolution, 0.0f, 320.0f, "%.0f");
    ImGui::TextWrapped("0 = off, 80 = strong jitter, 160 = subtle, 320 = barely visible");

    // -- Fog --
    ImGui::SeparatorText("Fog");
    ImGui::ColorEdit3("Fog Color", s.fog_color);
    ImGui::SliderFloat("Fog Start", &s.fog_start, 0.0f, 50.0f);
    ImGui::SliderFloat("Fog End", &s.fog_end, 1.0f, 100.0f);

    // -- Color & Post --
    ImGui::SeparatorText("Color & Post-Processing");
    ImGui::ColorEdit3("Clear Color", s.clear_color);
    ImGui::ColorEdit4("Tint", s.tint_color);
    ImGui::Checkbox("Dithering", &s.dithering_enabled);
    ImGui::SliderFloat("Color Depth", &s.color_depth, 3.0f, 255.0f, "%.0f");
    ImGui::TextWrapped("31 = PS1 (15-bit), 63 = 18-bit, 255 = full 24-bit");

    // -- Weapon --
    if (s.weapon_offset && s.weapon_rotation && s.weapon_scale) {
        ImGui::SeparatorText("Weapon");
        ImGui::DragFloat3("Offset", &s.weapon_offset->x, 0.01f, -3.0f, 3.0f, "%.2f");
        ImGui::DragFloat3("Rotation", &s.weapon_rotation->x, 1.0f, -180.0f, 180.0f, "%.1f");
        ImGui::DragFloat("Scale", s.weapon_scale, 0.01f, 0.01f, 3.0f, "%.2f");
    }

    // -- Help --
    ImGui::SeparatorText("Controls");
    ImGui::TextWrapped(
        "Tab = toggle this menu | ` = console | WASD = move | Mouse = look | Shift = sprint | Esc = quit");

    ImGui::End();
}

}  // namespace qp
