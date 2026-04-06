#include <cmath>
#include <engine/engine.h>
#include <game/dungeon/dungeon_map.h>
#include <game/weapon.h>

#include "engine/platform/input.h"
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
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#......##......#",
    "#..............#",
    "#..............#",
    "################",
};
// clang-format on

static constexpr i32 MAP_ROWS = sizeof(DUNGEON_MAP) / sizeof(DUNGEON_MAP[0]);

static const f32 MIN_Y = 2.0f;

int main()
{
    // -- Window --
    WindowConfig win_cfg;
    win_cfg.width     = 1280;
    win_cfg.height    = 720;
    win_cfg.title     = "quakepg debugger";
    win_cfg.vsync     = false;
    win_cfg.maximized = false;

    Window *window = window_create(win_cfg);
    if (!window) {
        return 1;
    }

    input_init(window);
    input_set_cursor_locked(true);
    timer_init();

    // -- ImGui --
    GLFWwindow *glfw_win = (GLFWwindow *)window_get_native_handle(window);
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    imgui_apply_theme();
    ImGui_ImplGlfw_InitForOpenGL(glfw_win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // -- Console & Debug UI --
    console_init();
    bool show_console  = false;
    bool show_debug    = false;
    bool cursor_locked = true;

    // -- Renderer --
    RendererConfig ren_cfg;
    ren_cfg.internal_width  = 640;
    ren_cfg.internal_height = 480;
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

    // -- Weapon --
    Weapon sword = weapon_create("assets/medievalweaponspack/MedievalWeaponsPack/Sword.glb");

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
    f32  vel_y       = 0.0f;
    f32  gravity     = 15.0f;
    bool on_ground   = true;

    // -- Debug UI state (shared with debug window) --
    DebugUIState dbg      = {};
    dbg.renderer          = renderer;
    dbg.internal_w        = 640;
    dbg.internal_h        = 480;
    dbg.resolution_preset = 2;
    dbg.camera            = &cam;
    dbg.base_speed        = 4.0f;
    dbg.gravity           = 15.0f;
    dbg.jump_speed        = 5.0f;
    dbg.sprint_mult       = 2.0f;
    dbg.snap_resolution   = 0;
    dbg.fog_color[0]      = 0.02f;
    dbg.fog_color[1]      = 0.01f;
    dbg.fog_color[2]      = 0.05f;
    dbg.clear_color[0]    = 0.02f;
    dbg.clear_color[1]    = 0.01f;
    dbg.clear_color[2]    = 0.05f;
    dbg.fog_start         = 5.0f;
    dbg.fog_end           = 40.0f;
    dbg.dithering_enabled = true;
    dbg.color_depth       = 31.0f;
    dbg.tint_color[0]     = 1.0f;
    dbg.tint_color[1]     = 1.0f;
    dbg.tint_color[2]     = 1.0f;
    dbg.tint_color[3]     = 1.0f;
    dbg.weapon_offset     = &sword.offset;
    dbg.weapon_rotation   = &sword.rotation;
    dbg.weapon_scale      = &sword.scale;

    LOG_INFO("=== QuakePG - Dungeon Crawler ===");
    LOG_INFO("WASD move, Mouse look, Shift sprint, ESC quit");

    // ===== GAME LOOP =====
    while (!window_should_close(window)) {
        input_update();
        timer_update();
        input_poll();

        f32 dt = (f32)timer_delta();
        if (dt > 0.1f) {
            dt = 0.1f;
        }

        // -- Input --
        if (input_key_pressed(Key::Escape)) {
            break;
        }

        if (input_key_pressed(Key::GraveAccent)) {
            show_console  = !show_console;
            cursor_locked = !show_console && !show_debug;
            input_set_cursor_locked(cursor_locked);
        }

        if (input_key_pressed(Key::Tab)) {
            show_debug    = !show_debug;
            cursor_locked = !show_debug && !show_console;
            input_set_cursor_locked(cursor_locked);
        }

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
        if (input_key_pressed(Key::Space) && on_ground) {
            vel_y = dbg.jump_speed;
        }

        // Attack with left mouse button
        if (cursor_locked && input_mouse_pressed(MouseButton::Left)) {
            weapon_attack(sword);
        }

        cam.speed = input_key_down(Key::LeftShift) ? dbg.base_speed * dbg.sprint_mult : dbg.base_speed;

        Vec3 fwd_dir   = camera_forward(cam);
        Vec3 fwd_flat  = vec3_normalize({fwd_dir.x, 0.0f, fwd_dir.z});
        Vec3 right_dir = camera_right(cam);

        vel_y -= dbg.gravity * dt;

        Vec3 move = fwd_flat * fwd + right_dir * right;
        if (vec3_length_sq(move) > 0.001f) {
            move = vec3_normalize(move);
        }

        Vec3 velocity = move * (cam.speed * dt);
        velocity.y    = vel_y * dt;
        on_ground     = false;

        AABB player_box = aabb_from_center_size(cam.position, player_half * 2.0f);

        Vec3 old_pos = cam.position;

        Vec3 new_pos =
            aabb_slide(player_box, velocity, dungeon.wall_colliders.data(), (u32)dungeon.wall_colliders.size());
        cam.position = new_pos;

        if (fabsf((new_pos.y) - (old_pos.y + velocity.y)) > 0.001f) {
            vel_y = 0.0f;
        }
        if (cam.position.y <= 2.0f) {
            cam.position.y = 2.0f;
            vel_y          = 0.0f;
            on_ground      = true;
        }

        LOG_INFO("vel_y: %.2f | new_pos.y: %.2f | expected: %.2f", vel_y, new_pos.y, old_pos.y + velocity.y);
        // -- Update weapon --
        f32 move_speed = vec3_length(velocity);
        weapon_update(sword, dt, move_speed);

        // -- Render --
        i32 fb_w, fb_h;
        window_get_framebuffer_size(window, &fb_w, &fb_h);
        i32 ren_w, ren_h;
        renderer_get_internal_size(renderer, &ren_w, &ren_h);
        f32 aspect = (f32)ren_w / (f32)ren_h;

        Mat4 view  = camera_view_matrix(cam);
        Mat4 proj  = camera_projection_matrix(cam, aspect);
        Mat4 model = mat4_identity();

        renderer_set_clear_color(dbg.clear_color[0], dbg.clear_color[1], dbg.clear_color[2], 1.0f);
        renderer_begin_frame(renderer);

        shader_bind(psx_shader);
        shader_set_mat4(psx_shader, "uView", view.data);
        shader_set_mat4(psx_shader, "uProjection", proj.data);
        shader_set_mat4(psx_shader, "uModel", model.data);
        shader_set_float(psx_shader, "uSnapResolution", dbg.snap_resolution);
        shader_set_vec3(psx_shader, "uFogColor", dbg.fog_color[0], dbg.fog_color[1], dbg.fog_color[2]);
        shader_set_int(psx_shader, "uDitheringEnabled", dbg.dithering_enabled ? 1 : 0);
        shader_set_int(psx_shader, "uTexture", 0);
        shader_set_vec4(
            psx_shader, "uTintColor", dbg.tint_color[0], dbg.tint_color[1], dbg.tint_color[2], dbg.tint_color[3]);
        shader_set_int(psx_shader, "uUseTexture", 1);
        shader_set_float(psx_shader, "uFogStart", dbg.fog_start);
        shader_set_float(psx_shader, "uFogEnd", dbg.fog_end);
        shader_set_float(psx_shader, "uColorDepth", dbg.color_depth);

        texture_bind(white_tex, 0);

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

        // -- Draw weapon (first-person, on top of world) --
        weapon_draw(sword, cam, psx_shader, aspect);

        shader_unbind();
        renderer_end_frame(renderer);
        renderer_present(renderer, fb_w, fb_h);

        // -- ImGui --
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        dbg.fb_w = fb_w;
        dbg.fb_h = fb_h;
        debug_ui_draw(dbg, &show_debug);
        console_draw(&show_console);

        if (!show_debug && !show_console && !cursor_locked) {
            cursor_locked = true;
            input_set_cursor_locked(true);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window_swap_buffers(window);
    }

    // -- Cleanup --
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    weapon_destroy(sword);
    dungeon_map_destroy(dungeon);
    texture_destroy(white_tex);
    texture_destroy(wall_tex);
    texture_destroy(floor_tex);
    texture_destroy(ceil_tex);
    shader_destroy(psx_shader);
    renderer_destroy(renderer);
    window_destroy(window);

    return 0;
}
