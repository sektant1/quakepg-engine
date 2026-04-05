#include <engine/renderer/renderer.h>
#include <engine/core/log.h>

#include <glad/glad.h>

namespace qp {

struct Renderer {
    u32 fbo;         // framebuffer object (PSX resolution)
    u32 fbo_texture; // color attachment
    u32 rbo;         // depth/stencil renderbuffer
    u32 screen_vao;  // fullscreen quad VAO
    u32 screen_vbo;
    i32 internal_w;
    i32 internal_h;
};

static void create_fullscreen_quad(Renderer* r) {
    // Fullscreen quad for FBO upscale
    float quad[] = {
        // pos      texcoord
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
    };

    glGenVertexArrays(1, &r->screen_vao);
    glGenBuffers(1, &r->screen_vbo);
    glBindVertexArray(r->screen_vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->screen_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

Renderer* renderer_create(const RendererConfig& config) {
    Renderer* r = new Renderer();
    r->internal_w = config.internal_width;
    r->internal_h = config.internal_height;

    // Create FBO for PSX internal resolution
    glGenFramebuffers(1, &r->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, r->fbo);

    // Color texture
    glGenTextures(1, &r->fbo_texture);
    glBindTexture(GL_TEXTURE_2D, r->fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, r->internal_w, r->internal_h,
                 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, r->fbo_texture, 0);

    // Depth/stencil renderbuffer
    glGenRenderbuffers(1, &r->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, r->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                          r->internal_w, r->internal_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, r->rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    create_fullscreen_quad(r);

    glEnable(GL_DEPTH_TEST);

    LOG_INFO("Renderer created: internal %dx%d", r->internal_w, r->internal_h);

    return r;
}

void renderer_destroy(Renderer* r) {
    if (r) {
        glDeleteFramebuffers(1, &r->fbo);
        glDeleteTextures(1, &r->fbo_texture);
        glDeleteRenderbuffers(1, &r->rbo);
        glDeleteVertexArrays(1, &r->screen_vao);
        glDeleteBuffers(1, &r->screen_vbo);
        delete r;
    }
}

void renderer_begin_frame(Renderer* r) {
    // Render to PSX-resolution FBO
    glBindFramebuffer(GL_FRAMEBUFFER, r->fbo);
    glViewport(0, 0, r->internal_w, r->internal_h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_end_frame(Renderer* r) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderer_present(Renderer* r, i32 window_width, i32 window_height) {
    // Blit FBO to screen with nearest-neighbor (pixelated upscale)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, r->fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, r->internal_w, r->internal_h,
        0, 0, window_width, window_height,
        GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderer_resize(Renderer* r, i32 width, i32 height) {
    if (width == r->internal_w && height == r->internal_h) return;
    if (width < 1 || height < 1) return;

    r->internal_w = width;
    r->internal_h = height;

    // Resize FBO color texture
    glBindTexture(GL_TEXTURE_2D, r->fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // Resize depth/stencil renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, r->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    LOG_INFO("Renderer resized: %dx%d", width, height);
}

void renderer_get_internal_size(Renderer* r, i32* width, i32* height) {
    *width  = r->internal_w;
    *height = r->internal_h;
}

void renderer_set_clear_color(f32 r, f32 g, f32 b, f32 a) {
    glClearColor(r, g, b, a);
}

void renderer_set_viewport(i32 x, i32 y, i32 w, i32 h) {
    glViewport(x, y, w, h);
}

} // namespace qp
