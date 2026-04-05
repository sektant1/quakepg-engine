#include <engine/engine.h>

int main()
{
    // Create window
    qp::WindowConfig win_cfg;
    win_cfg.width  = 960;
    win_cfg.height = 720;
    win_cfg.title  = "QuakePG - PSX Dungeon Crawler";
    win_cfg.vsync  = true;

    qp::Window *window = qp::window_create(win_cfg);
    if (!window) {
        return 1;
    }

    // Init subsystems
    qp::input_init(window);
    qp::timer_init();

    // Create renderer with PSX internal resolution
    qp::RendererConfig ren_cfg;
    ren_cfg.internal_width  = 320;
    ren_cfg.internal_height = 240;

    qp::Renderer *renderer = qp::renderer_create(ren_cfg);
    if (!renderer) {
        qp::window_destroy(window);
        return 1;
    }

    qp::renderer_set_clear_color(0.05f, 0.02f, 0.08f, 1.0f);

    // Load basic shader
    qp::Shader shader = qp::shader_load("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    if (!shader.program) {
        LOG_ERROR("Failed to load shaders");
        qp::renderer_destroy(renderer);
        qp::window_destroy(window);
        return 1;
    }

    // Create test triangle
    qp::Mesh triangle = qp::mesh_create_triangle();

    LOG_INFO("Game initialized. Press ESC to quit.");

    // Game loop
    while (!qp::window_should_close(window)) {
        qp::input_update();
        qp::timer_update();
        qp::input_poll();

        // Handle input
        if (qp::input_key_pressed(qp::Key::Escape)) {
            break;
        }

        // Render to PSX FBO
        qp::renderer_begin_frame(renderer);

        qp::shader_bind(shader);
        qp::mesh_draw(triangle);
        qp::shader_unbind();

        qp::renderer_end_frame(renderer);

        // Upscale to window
        qp::i32 w, h;
        qp::window_get_framebuffer_size(window, &w, &h);
        qp::renderer_present(renderer, w, h);

        qp::window_swap_buffers(window);
    }

    // Cleanup
    qp::mesh_destroy(triangle);
    qp::shader_destroy(shader);
    qp::renderer_destroy(renderer);
    qp::window_destroy(window);

    LOG_INFO("Game shutdown complete.");
    return 0;
}
