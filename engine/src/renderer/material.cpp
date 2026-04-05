#include <engine/renderer/material.h>

namespace qp {

void material_bind(const Material& mat) {
    shader_bind(mat.shader);
    texture_bind(mat.texture, 0);
    shader_set_int(mat.shader, "uTexture", 0);
    shader_set_vec4(mat.shader, "uTintColor", mat.color.x, mat.color.y, mat.color.z, mat.color.w);
    shader_set_int(mat.shader, "uUseTexture", mat.use_texture ? 1 : 0);
}

void material_unbind() {
    texture_unbind(0);
    shader_unbind();
}

void material_destroy(Material& mat) {
    texture_destroy(mat.texture);
    // shader not owned by material - destroyed separately
}

} // namespace qp
