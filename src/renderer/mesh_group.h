#pragma once

#include <vector>

#include "../gamedefine.h"
#include "mesh.h"
#include "texture.h"

class aiMesh;

struct mesh_group_t
{
    std::vector<mesh_t>     meshes;
    std::vector<texture_t>  textures;
    std::vector<u16>        mesh_to_texture;

    void render();

    void clear();

    static mesh_group_t assimp_load(const char* file_name);

private:
    static mesh_t assimp_load_mesh_helper(size_t mesh_index, aiMesh* mesh_node);

};
