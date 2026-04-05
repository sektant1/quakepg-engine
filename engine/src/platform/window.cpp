#include <engine/platform/window.h>
#include <engine/core/log.h>
#include <engine/core/assert.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace qp {

struct Window {
    GLFWwindow* handle;
    i32 width;
    i32 height;
};

static bool s_glfw_initialized = false;

static void glfw_error_callback(int error, const char* description) {
    LOG_ERROR("GLFW error %d: %s", error, description);
}

Window* window_create(const WindowConfig& config) {
    if (!s_glfw_initialized) {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            LOG_FATAL("Failed to initialize GLFW");
            return nullptr;
        }
        s_glfw_initialized = true;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* handle = glfwCreateWindow(
        config.width, config.height, config.title, nullptr, nullptr);

    if (!handle) {
        LOG_FATAL("Failed to create GLFW window");
        return nullptr;
    }

    glfwMakeContextCurrent(handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_FATAL("Failed to initialize glad");
        glfwDestroyWindow(handle);
        return nullptr;
    }

    glfwSwapInterval(config.vsync ? 1 : 0);

    Window* w = new Window();
    w->handle = handle;
    w->width = config.width;
    w->height = config.height;

    LOG_INFO("Window created: %dx%d - %s", config.width, config.height,
             config.title);
    LOG_INFO("OpenGL: %s", glGetString(GL_VERSION));
    LOG_INFO("Renderer: %s", glGetString(GL_RENDERER));

    return w;
}

void window_destroy(Window* w) {
    if (w) {
        glfwDestroyWindow(w->handle);
        delete w;
    }
    if (s_glfw_initialized) {
        glfwTerminate();
        s_glfw_initialized = false;
    }
}

bool window_should_close(Window* w) {
    return glfwWindowShouldClose(w->handle);
}

void window_swap_buffers(Window* w) {
    glfwSwapBuffers(w->handle);
}

void window_get_size(Window* w, i32* width, i32* height) {
    glfwGetWindowSize(w->handle, width, height);
}

void window_get_framebuffer_size(Window* w, i32* width, i32* height) {
    glfwGetFramebufferSize(w->handle, width, height);
}

void* window_get_native_handle(Window* w) {
    return w->handle;
}

} // namespace qp
