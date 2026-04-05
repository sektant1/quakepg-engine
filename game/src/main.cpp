#include <engine/engine.h>
#include <game/dungeon/dungeon_map.h>

#include "engine/core/log.h"

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

        camera_process_mouse(cam, input_mouse_dx(), input_mouse_dy());

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

        renderer_begin_frame(renderer);

        shader_bind(psx_shader);
        shader_set_mat4(psx_shader, "uView", view.data);
        shader_set_mat4(psx_shader, "uProjection", proj.data);
        shader_set_mat4(psx_shader, "uModel", model.data);
        shader_set_float(psx_shader, "uSnapResolution", 160.0f);
        shader_set_vec3(psx_shader, "uFogColor", 0.02f, 0.01f, 0.05f);
        shader_set_int(psx_shader, "uDitheringEnabled", 1);
        shader_set_int(psx_shader, "uTexture", 0);
        shader_set_vec4(psx_shader, "uTintColor", 1, 1, 1, 1);
        shader_set_int(psx_shader, "uUseTexture", 1);

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
        window_swap_buffers(window);
    }

    // -- Cleanup --
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
