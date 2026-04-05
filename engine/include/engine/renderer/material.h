#pragma once

#include <engine/core/types.h>
#include <engine/core/math.h>
#include <engine/renderer/shader.h>
#include <engine/renderer/texture.h>

namespace qp {

struct Material {
    Shader  shader;
    Texture texture;
    Vec4    color;          // tint multiplier
    bool    use_texture;    // false = vertex color only (very PSX)
};

inline Material material_create(Shader shader, Texture texture) {
    return {shader, texture, {1,1,1,1}, true};
}

inline Material material_create_colored(Shader shader, Vec4 color) {
    Texture white = texture_create_white();
    return {shader, white, color, false};
}

void material_bind(const Material& mat);
void material_unbind();
void material_destroy(Material& mat);

} // namespace qp
