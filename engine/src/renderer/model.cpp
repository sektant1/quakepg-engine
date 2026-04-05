#include <engine/renderer/model.h>
#include <engine/core/log.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>
#include <stb_image.h>

#include <cstring>
#include <string>
#include <vector>

namespace qp {

// ============================================================================
// Helpers
// ============================================================================

static std::string directory_of(const char *path)
{
    std::string s(path);
    auto pos = s.find_last_of("/\\");
    if (pos != std::string::npos) {
        return s.substr(0, pos + 1);
    }
    return "./";
}

static Texture load_embedded_texture(const aiTexture *ai_tex)
{
    Texture tex = {};

    // Compressed texture (e.g. PNG/JPG stored in memory)
    if (ai_tex->mHeight == 0) {
        int w, h, ch;
        unsigned char *data = stbi_load_from_memory(
            reinterpret_cast<const unsigned char *>(ai_tex->pcData),
            (int)ai_tex->mWidth, &w, &h, &ch, 4);

        if (!data) {
            LOG_WARN("Failed to decode embedded texture");
            return tex;
        }

        glGenTextures(1, &tex.id);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        tex.width    = w;
        tex.height   = h;
        tex.channels = ch;
        stbi_image_free(data);
    }

    return tex;
}

static Mesh process_mesh(const aiMesh *ai_mesh)
{
    std::vector<Vertex> vertices(ai_mesh->mNumVertices);
    std::vector<u32>    indices;

    for (u32 i = 0; i < ai_mesh->mNumVertices; i++) {
        Vertex &v = vertices[i];

        v.position[0] = ai_mesh->mVertices[i].x;
        v.position[1] = ai_mesh->mVertices[i].y;
        v.position[2] = ai_mesh->mVertices[i].z;

        if (ai_mesh->mTextureCoords[0]) {
            v.texcoord[0] = ai_mesh->mTextureCoords[0][i].x;
            v.texcoord[1] = ai_mesh->mTextureCoords[0][i].y;
        } else {
            v.texcoord[0] = 0.0f;
            v.texcoord[1] = 0.0f;
        }

        if (ai_mesh->mNormals) {
            v.normal[0] = ai_mesh->mNormals[i].x;
            v.normal[1] = ai_mesh->mNormals[i].y;
            v.normal[2] = ai_mesh->mNormals[i].z;
        } else {
            v.normal[0] = 0.0f;
            v.normal[1] = 1.0f;
            v.normal[2] = 0.0f;
        }

        if (ai_mesh->mColors[0]) {
            v.color[0] = ai_mesh->mColors[0][i].r;
            v.color[1] = ai_mesh->mColors[0][i].g;
            v.color[2] = ai_mesh->mColors[0][i].b;
            v.color[3] = ai_mesh->mColors[0][i].a;
        } else {
            v.color[0] = 1.0f;
            v.color[1] = 1.0f;
            v.color[2] = 1.0f;
            v.color[3] = 1.0f;
        }
    }

    for (u32 i = 0; i < ai_mesh->mNumFaces; i++) {
        const aiFace &face = ai_mesh->mFaces[i];
        for (u32 j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return mesh_create(vertices.data(), (u32)vertices.size(),
                       indices.data(), (u32)indices.size());
}

// ============================================================================
// Public API
// ============================================================================

Model model_load(const char *path)
{
    Model model = {};

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        LOG_ERROR("Assimp: %s", importer.GetErrorString());
        return model;
    }

    std::string dir = directory_of(path);

    for (u32 i = 0; i < scene->mNumMeshes && model.mesh_count < MODEL_MAX_MESHES; i++) {
        ModelMesh &mm  = model.meshes[model.mesh_count];
        mm.mesh        = process_mesh(scene->mMeshes[i]);
        mm.has_texture = false;
        mm.texture     = {};

        // Try to load diffuse texture
        u32 mat_idx = scene->mMeshes[i]->mMaterialIndex;
        if (mat_idx < scene->mNumMaterials) {
            aiMaterial *mat = scene->mMaterials[mat_idx];
            aiString tex_path;

            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS ||
                mat->GetTexture(aiTextureType_BASE_COLOR, 0, &tex_path) == AI_SUCCESS) {
                // Try embedded texture first (works for GLB)
                const aiTexture *embedded = scene->GetEmbeddedTexture(tex_path.C_Str());
                if (embedded) {
                    mm.texture     = load_embedded_texture(embedded);
                    mm.has_texture = (mm.texture.id != 0);
                } else if (tex_path.data[0] == '*') {
                    int tex_idx = atoi(tex_path.data + 1);
                    if (tex_idx >= 0 && (u32)tex_idx < scene->mNumTextures) {
                        mm.texture     = load_embedded_texture(scene->mTextures[tex_idx]);
                        mm.has_texture = (mm.texture.id != 0);
                    }
                } else {
                    // External texture file
                    std::string full_path = dir + tex_path.C_Str();
                    mm.texture     = texture_load(full_path.c_str());
                    mm.has_texture = (mm.texture.id != 0);
                }
            }
        }

        model.mesh_count++;
    }

    LOG_INFO("Model loaded: %s (%u meshes)", path, model.mesh_count);
    return model;
}

void model_draw(const Model &model)
{
    for (u32 i = 0; i < model.mesh_count; i++) {
        const ModelMesh &mm = model.meshes[i];
        if (mm.has_texture) {
            texture_bind(mm.texture, 0);
        }
        mesh_draw(mm.mesh);
    }
}

void model_destroy(Model &model)
{
    for (u32 i = 0; i < model.mesh_count; i++) {
        mesh_destroy(model.meshes[i].mesh);
        if (model.meshes[i].has_texture) {
            texture_destroy(model.meshes[i].texture);
        }
    }
    model.mesh_count = 0;
}

} // namespace qp
