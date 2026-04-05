#include <engine/engine.h>
#include <game/dungeon/dungeon_map.h>

#include "engine/core/log.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace qp;

// ============================================================================
// Dungeon layout
// ============================================================================
// # = wall, . = open space, P = player start
// Each cell is CELL_SIZE x CELL_SIZE world units

// clang-format off
static const char* DUNGEON_MAP[] = {
    "################",
    "#......P.......#",
    "#..............#",
    "#..####..####..#",
    "#..#........#..#",
    "#..#........#..#",
    "#..#........#..#",
    "#..####..####..#",
    "#..............#",
    "#.....####.....#",
    "#.....#..#.....#",
    "#.....#..#.....#",
    "#.....####.....#",
    "#..............#",
    "#..##......##..#",
    "#..##......##..#",
    "#..............#",
    "#..............#",
    "################",
};
// clang-format on

static constexpr i32 MAP_ROWS = sizeof(DUNGEON_MAP) / sizeof(DUNGEON_MAP[0]);

int main()
{
    // -- Window --
    WindowConfig win_cfg;
    win_cfg.width  = 960;
    win_cfg.height = 720;
    win_cfg.title  = "QuakePG - PSX Dungeon Crawler";
    win_cfg.vsync  = true;

    Window *window = window_create(win_cfg);
    if (!window) {
        return 1;
    }

    input_init(window);
    input_set_cursor_locked(true);
    timer_init();

    // -- ImGui --
    GLFWwindow* glfw_win = (GLFWwindow*)window_get_native_handle(window);
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(glfw_win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // -- Debug UI state --
    bool show_debug     = false;
    bool cursor_locked  = true;

    // -- PSX tweakables --
    float snap_resolution   = 160.0f;
    float fog_color[3]      = {0.02f, 0.01f, 0.05f};
    float clear_color[3]    = {0.02f, 0.01f, 0.05f};
    float fog_start         = 5.0f;
    float fog_end           = 40.0f;
    bool  dithering_enabled = true;
    float tint_color[4]     = {1.0f, 1.0f, 1.0f, 1.0f};

    // -- Renderer --
    RendererConfig ren_cfg;
    ren_cfg.internal_width  = 320;
    ren_cfg.internal_height = 240;
    Renderer *renderer      = renderer_create(ren_cfg);
    renderer_set_clear_color(0.02f, 0.01f, 0.05f, 1.0f);

    // -- PSX Shader --
    Shader psx_shader = shader_load("assets/shaders/psx.vert", "assets/shaders/psx.frag");
    if (!psx_shader.program) {
        LOG_FATAL("Failed to load PSX shaders");
        return 1;
    }

    Texture white_tex = texture_create_white();
    Texture wall_tex  = texture_load("assets/textures/Brick_0.png");
    Texture floor_tex = texture_load("assets/textures/Cobble.png");
    Texture ceil_tex  = texture_load("assets/textures/Cobble_Ceiling.png");

    // -- Load dungeon --
    DungeonMap dungeon;
    dungeon_map_load(dungeon, DUNGEON_MAP, MAP_ROWS, 3.0f, 4.0f);

    // -- Camera at player spawn --
    Camera cam;
    cam.position = dungeon.player_spawn;
    cam.fov      = 90.0f;
    cam.speed    = 4.0f;

    // Player collision size (thin box, eye height ~1.6)
    Vec3 player_half = {0.3f, 0.8f, 0.3f};

    LOG_INFO("=== QuakePG - Dungeon Crawler ===");
    LOG_INFO("WASD move, Mouse look, Shift sprint, ESC quit");

    // ===== GAME LOOP =====
    while (!window_should_close(window)) {
        input_update();
        timer_update();
        input_poll();

        f32 dt = (f32)timer_delta();
        if (dt > 0.1f) {
            dt = 0.1f;  // cap for alt-tab
        }

        // -- Input --
        if (input_key_pressed(Key::Escape)) {
            LOG_FATAL("Closing the game");
            break;
        }

        // Toggle debug UI with Tab
        if (input_key_pressed(Key::Tab)) {
            show_debug = !show_debug;
            cursor_locked = !show_debug;
            input_set_cursor_locked(cursor_locked);
        }

        // Only process game input when cursor is locked (no debug UI)
        if (cursor_locked) {
            camera_process_mouse(cam, input_mouse_dx(), input_mouse_dy());
        }

        f32 fwd = 0, right = 0;
        if (input_key_down(Key::W)) {
            fwd += 1.0f;
        }
        if (input_key_down(Key::S)) {
            fwd -= 1.0f;
        }
        if (input_key_down(Key::D)) {
            right += 1.0f;
        }
        if (input_key_down(Key::A)) {
            right -= 1.0f;
        }

        cam.speed = input_key_down(Key::LeftShift) ? 8.0f : 4.0f;

        // Calculate desired velocity
        Vec3 fwd_dir   = camera_forward(cam);
        Vec3 fwd_flat  = vec3_normalize({fwd_dir.x, 0.0f, fwd_dir.z});
        Vec3 right_dir = camera_right(cam);

        Vec3 move = fwd_flat * fwd + right_dir * right;
        if (vec3_length_sq(move) > 0.001f) {
            move = vec3_normalize(move);
        }
        Vec3 velocity = move * (cam.speed * dt);

        // -- Collision with walls (slide) --
        AABB player_box = aabb_from_center_size(cam.position, player_half * 2.0f);
        Vec3 new_pos =
            aabb_slide(player_box, velocity, dungeon.wall_colliders.data(), (u32)dungeon.wall_colliders.size());
        cam.position = new_pos;

        // -- Render --
        i32 fb_w, fb_h;
        window_get_framebuffer_size(window, &fb_w, &fb_h);
        f32 aspect = (f32)ren_cfg.internal_width / (f32)ren_cfg.internal_height;

        Mat4 view  = camera_view_matrix(cam);
        Mat4 proj  = camera_projection_matrix(cam, aspect);
        Mat4 model = mat4_identity();

        renderer_set_clear_color(clear_color[0], clear_color[1], clear_color[2], 1.0f);
        renderer_begin_frame(renderer);

        shader_bind(psx_shader);
        shader_set_mat4(psx_shader, "uView", view.data);
        shader_set_mat4(psx_shader, "uProjection", proj.data);
        shader_set_mat4(psx_shader, "uModel", model.data);
        shader_set_float(psx_shader, "uSnapResolution", snap_resolution);
        shader_set_vec3(psx_shader, "uFogColor", fog_color[0], fog_color[1], fog_color[2]);
        shader_set_int(psx_shader, "uDitheringEnabled", dithering_enabled ? 1 : 0);
        shader_set_int(psx_shader, "uTexture", 0);
        shader_set_vec4(psx_shader, "uTintColor", tint_color[0], tint_color[1], tint_color[2], tint_color[3]);
        shader_set_int(psx_shader, "uUseTexture", 1);
        shader_set_float(psx_shader, "uFogStart", fog_start);
        shader_set_float(psx_shader, "uFogEnd", fog_end);

        texture_bind(white_tex, 0);

        // Draw dungeon
        if (dungeon.floor_mesh.vao) {
            texture_bind(floor_tex, 0);
            mesh_draw(dungeon.floor_mesh);
        }
        if (dungeon.wall_mesh.vao) {
            texture_bind(wall_tex, 0);
            mesh_draw(dungeon.wall_mesh);
        }
        if (dungeon.ceiling_mesh.vao) {
            texture_bind(floor_tex, 0);
            mesh_draw(dungeon.ceiling_mesh);
        }

        shader_unbind();
        renderer_end_frame(renderer);
        renderer_present(renderer, fb_w, fb_h);

        // -- ImGui (renders at native window resolution, on top of PSX framebuffer) --
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_debug) {
            ImGui::Begin("PSX Debug", &show_debug);

            ImGui::SeparatorText("Performance");
            ImGui::Text("FPS: %u", timer_fps());
            ImGui::Text("Frame: %.2f ms", (float)timer_delta() * 1000.0f);
            ImGui::Text("Pos: %.1f, %.1f, %.1f", cam.position.x, cam.position.y, cam.position.z);

            ImGui::SeparatorText("Camera");
            ImGui::SliderFloat("FOV", &cam.fov, 60.0f, 120.0f);
            ImGui::SliderFloat("Speed", &cam.speed, 1.0f, 20.0f);
            ImGui::SliderFloat("Sensitivity", &cam.sensitivity, 0.05f, 0.5f);

            ImGui::SeparatorText("Vertex Snapping");
            ImGui::SliderFloat("Snap Resolution", &snap_resolution, 0.0f, 320.0f, "%.0f");
            ImGui::TextWrapped("0 = off, 80 = strong jitter, 160 = subtle, 320 = barely visible");

            ImGui::SeparatorText("Fog");
            ImGui::ColorEdit3("Fog Color", fog_color);
            ImGui::SliderFloat("Fog Start", &fog_start, 0.0f, 50.0f);
            ImGui::SliderFloat("Fog End", &fog_end, 1.0f, 100.0f);

            ImGui::SeparatorText("Color");
            ImGui::ColorEdit3("Clear Color", clear_color);
            ImGui::ColorEdit4("Tint", tint_color);
            ImGui::Checkbox("Dithering", &dithering_enabled);

            ImGui::SeparatorText("Controls");
            ImGui::TextWrapped("Tab = toggle this menu | WASD = move | Mouse = look | Shift = sprint | Esc = quit");

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window_swap_buffers(window);
    }

    // -- Cleanup --
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    dungeon_map_destroy(dungeon);
    texture_destroy(white_tex);
    texture_destroy(wall_tex);
    texture_destroy(floor_tex);
    texture_destroy(ceil_tex);
    shader_destroy(psx_shader);
    renderer_destroy(renderer);
    window_destroy(window);

    LOG_INFO("Game shutdown complete.");
    return 0;
}
