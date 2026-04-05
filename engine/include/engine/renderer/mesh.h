#pragma once

#include <engine/core/types.h>

namespace qp {

struct Vertex {
    f32 position[3];
    f32 texcoord[2];
    f32 normal[3];
    f32 color[4];
};

struct Mesh {
    u32 vao;
    u32 vbo;
    u32 ebo;
    u32 index_count;
};

Mesh mesh_create(const Vertex* vertices, u32 vert_count,
                 const u32* indices, u32 idx_count);
void mesh_destroy(Mesh& m);
void mesh_draw(const Mesh& m);

// Helpers for quick testing
Mesh mesh_create_triangle();
Mesh mesh_create_cube();

} // namespace qp
