#pragma once

#include <engine/core/types.h>

namespace qp {

struct Texture {
    u32 id;
    i32 width;
    i32 height;
    i32 channels;
};

// Load texture with GL_NEAREST filtering (PSX style)
Texture texture_load(const char* path);

// Create 1x1 white texture (fallback)
Texture texture_create_white();

void texture_bind(const Texture& t, u32 slot);
void texture_unbind(u32 slot);
void texture_destroy(Texture& t);

} // namespace qp
