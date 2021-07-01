#include "mesh_group.h"
#include "texture.h"
#include "../kc_math.h"
#include "../runtime/timer.h"
#include "../modules/console.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void mesh_group_t::render()
{
    for(size_t i = 0; i < meshes.size(); ++i)
    {
        uint16 mat_index = mesh_to_texture[i];
        if(mat_index < textures.size() && textures[mat_index].texture_id != 0)
        {
            textures[mat_index].gl_use_texture();
        }

        meshes[i].gl_render_mesh();
    }
}

void mesh_group_t::clear()
{
    for(size_t i = 0; i < meshes.size(); ++i)
    {
        mesh_t::gl_delete_mesh(meshes[i]);
    }
    for(size_t i = 0; i < textures.size(); ++i)
    {
        texture_t::gl_delete(textures[i]);
    }
}

void mesh_group_t::assimp_load(const char* file_name)
{
    timer::timestamp();

    Assimp::Importer importer;
    /*  NOTE: To create smooth normals respecting edges sharper than a given angle,
        use importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90) along with
        aiProcess_GenSmoothNormals flag. https://github.com/assimp/assimp/issues/1713 */
    const aiScene* scene = importer.ReadFile(file_name,
                                             aiProcess_Triangulate
                                             |aiProcess_GenSmoothNormals
                                             |aiProcess_JoinIdenticalVertices
    );
    if(!scene)
    {
        console_printf("Model '%s' failed to load: %s\n", file_name, importer.GetErrorString());
        return;
    }
    console_printf("took %f seconds to Importer::ReadFile\n", timer::timestamp());

    meshes.resize(scene->mNumMeshes);
    mesh_to_texture.resize(scene->mNumMeshes);
    textures.resize(scene->mNumMaterials);

    console_printf("took %f seconds to resize 3 vectors\n", timer::timestamp());

    // Unpack meshes
    for(size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        assimp_load_mesh_helper(i, scene->mMeshes[i]);
    }

    console_printf("took %f seconds to unpack all the meshes\n", timer::timestamp());

    // Load diffuse textures
    for(size_t i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* mat = scene->mMaterials[i];
        if(mat->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString path;
            if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                int idx = (int)std::string(path.data).find_last_of("\\");
                std::string texture_file_name = std::string(path.data).substr(idx+1);
                std::string model_file_directory = std::string(file_name);
                idx = max((int)model_file_directory.find_last_of("/"), (int)model_file_directory.find_last_of("\\"));
                model_file_directory = model_file_directory.substr(0, idx + 1);

                std::string tex_path = model_file_directory + texture_file_name;
                texture_t::gl_create_from_file(textures[i], tex_path.c_str());
            }
        }
    }

    console_printf("took %f seconds to load all the textures\n", timer::timestamp());
}

void mesh_group_t::assimp_load_mesh_helper(size_t mesh_index, aiMesh* mesh_node)
{
    const uint8 vb_entries_per_vertex = 8;
    std::vector<real32> vb(mesh_node->mNumVertices * vb_entries_per_vertex);
    std::vector<uint32> ib(mesh_node->mNumFaces * mesh_node->mFaces[0].mNumIndices);
    if(mesh_node->mTextureCoords[0])
    {
        for(size_t i = 0; i < mesh_node->mNumVertices; ++i)
        {
            // mNormals and mVertices are both mNumVertices in size
            size_t v_start_index = i * vb_entries_per_vertex;
            vb[v_start_index] = mesh_node->mVertices[i].x;
            vb[v_start_index + 1] = mesh_node->mVertices[i].y;
            vb[v_start_index + 2] = mesh_node->mVertices[i].z;
            vb[v_start_index + 3] = mesh_node->mTextureCoords[0][i].x;
            vb[v_start_index + 4] = mesh_node->mTextureCoords[0][i].y;
            vb[v_start_index + 5] = mesh_node->mNormals[i].x;
            vb[v_start_index + 6] = mesh_node->mNormals[i].y;
            vb[v_start_index + 7] = mesh_node->mNormals[i].z;
        }
    }
    else
    {
        for(size_t i = 0; i < mesh_node->mNumVertices; ++i)
        {
            size_t v_start_index = i * vb_entries_per_vertex;
            vb[v_start_index] = mesh_node->mVertices[i].x;
            vb[v_start_index + 1] = mesh_node->mVertices[i].y;
            vb[v_start_index + 2] = mesh_node->mVertices[i].z;
            vb[v_start_index + 3] = 0.f;
            vb[v_start_index + 4] = 0.f;
            vb[v_start_index + 5] = mesh_node->mNormals[i].x;
            vb[v_start_index + 6] = mesh_node->mNormals[i].y;
            vb[v_start_index + 7] = mesh_node->mNormals[i].z;
        }
    }

    for(size_t i = 0; i < mesh_node->mNumFaces; ++i)
    {
        aiFace face = mesh_node->mFaces[i];
        for(size_t j = 0; j < face.mNumIndices; ++j)
        {
            ib[i * face.mNumIndices + j] = face.mIndices[j]; // prob sometimes not correct to index ib this way
        }
    }

    mesh_t mesh;
    mesh_t::gl_create_mesh(mesh, &vb[0], &ib[0], (uint32)vb.size(), (uint32)ib.size());
    meshes[mesh_index] = mesh;
    mesh_to_texture[mesh_index] = mesh_node->mMaterialIndex;
}