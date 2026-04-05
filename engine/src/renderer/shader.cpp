#include <engine/renderer/shader.h>
#include <engine/core/log.h>

#include <glad/glad.h>
#include <cstdio>
#include <cstdlib>

namespace qp {

static char* read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        LOG_ERROR("Failed to open file: %s", path);
        return nullptr;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

static u32 compile_shader(GLenum type, const char* source) {
    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOG_ERROR("Shader compilation failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

Shader shader_create(const char* vertex_src, const char* fragment_src) {
    Shader s = {};

    u32 vert = compile_shader(GL_VERTEX_SHADER, vertex_src);
    u32 frag = compile_shader(GL_FRAGMENT_SHADER, fragment_src);

    if (!vert || !frag) {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return s;
    }

    s.program = glCreateProgram();
    glAttachShader(s.program, vert);
    glAttachShader(s.program, frag);
    glLinkProgram(s.program);

    int success;
    glGetProgramiv(s.program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(s.program, sizeof(log), nullptr, log);
        LOG_ERROR("Shader link failed: %s", log);
        glDeleteProgram(s.program);
        s.program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return s;
}

Shader shader_load(const char* vert_path, const char* frag_path) {
    char* vert_src = read_file(vert_path);
    char* frag_src = read_file(frag_path);

    Shader s = {};
    if (vert_src && frag_src) {
        s = shader_create(vert_src, frag_src);
    }

    free(vert_src);
    free(frag_src);
    return s;
}

void shader_destroy(Shader& s) {
    if (s.program) {
        glDeleteProgram(s.program);
        s.program = 0;
    }
}

void shader_bind(const Shader& s)   { glUseProgram(s.program); }
void shader_unbind()                 { glUseProgram(0); }

void shader_set_int(const Shader& s, const char* name, i32 value) {
    glUniform1i(glGetUniformLocation(s.program, name), value);
}

void shader_set_float(const Shader& s, const char* name, f32 value) {
    glUniform1f(glGetUniformLocation(s.program, name), value);
}

void shader_set_vec3(const Shader& s, const char* name, f32 x, f32 y, f32 z) {
    glUniform3f(glGetUniformLocation(s.program, name), x, y, z);
}

void shader_set_vec4(const Shader& s, const char* name, f32 x, f32 y, f32 z, f32 w) {
    glUniform4f(glGetUniformLocation(s.program, name), x, y, z, w);
}

void shader_set_mat4(const Shader& s, const char* name, const f32* data) {
    glUniformMatrix4fv(glGetUniformLocation(s.program, name), 1, GL_FALSE, data);
}

} // namespace qp
