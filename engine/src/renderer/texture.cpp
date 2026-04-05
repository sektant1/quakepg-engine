#include <engine/renderer/texture.h>
#include <engine/core/log.h>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace qp {

Texture texture_load(const char* path) {
    Texture t = {};

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &t.width, &t.height, &t.channels, 0);

    if (!data) {
        LOG_ERROR("Failed to load texture: %s", path);
        return t;
    }

    GLenum format = GL_RGB;
    if (t.channels == 1)      format = GL_RED;
    else if (t.channels == 3) format = GL_RGB;
    else if (t.channels == 4) format = GL_RGBA;

    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, t.width, t.height, 0,
                 format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // PSX style: nearest neighbor, mipmaps reduce aliasing at distance
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    LOG_INFO("Texture loaded: %s (%dx%d, %dch)", path, t.width, t.height, t.channels);
    return t;
}

Texture texture_create_white() {
    Texture t = {};
    t.width = 1;
    t.height = 1;
    t.channels = 4;

    u8 white[] = {255, 255, 255, 255};

    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    return t;
}

void texture_bind(const Texture& t, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, t.id);
}

void texture_unbind(u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void texture_destroy(Texture& t) {
    if (t.id) {
        glDeleteTextures(1, &t.id);
        t.id = 0;
    }
}

} // namespace qp
