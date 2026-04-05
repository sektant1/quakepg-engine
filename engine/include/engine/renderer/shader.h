#pragma once

#include <engine/core/types.h>

namespace qp {

struct Shader {
    u32 program;
};

Shader shader_create(const char* vertex_src, const char* fragment_src);
Shader shader_load(const char* vert_path, const char* frag_path);
void   shader_destroy(Shader& s);
void   shader_bind(const Shader& s);
void   shader_unbind();

void shader_set_int(const Shader& s, const char* name, i32 value);
void shader_set_float(const Shader& s, const char* name, f32 value);
void shader_set_vec3(const Shader& s, const char* name, f32 x, f32 y, f32 z);
void shader_set_vec4(const Shader& s, const char* name, f32 x, f32 y, f32 z, f32 w);
void shader_set_mat4(const Shader& s, const char* name, const f32* data);

} // namespace qp
