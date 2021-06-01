internal void render_meshgroup(meshgroup_t& meshgroup)
{
    for(size_t i = 0; i < meshgroup.meshes.size(); ++i)
    {
        uint16 mat_index = meshgroup.mesh_to_texture[i];
        if(mat_index < meshgroup.textures.size() && meshgroup.textures[mat_index].texture_id != 0)
        {
            gl_use_texture(meshgroup.textures[mat_index]);
        }

        gl_render_mesh(meshgroup.meshes[i]);
    }
}

internal void clear_meshgroup(meshgroup_t& meshgroup)
{
    for(size_t i = 0; i < meshgroup.meshes.size(); ++i)
    {
        gl_delete_mesh(meshgroup.meshes[i]);
    }
    for(size_t i = 0; i < meshgroup.textures.size(); ++i)
    {
        gl_delete_texture(meshgroup.textures[i]);
    }
}

internal void __assimp_load_mesh_helper(meshgroup_t& meshgroup, size_t mesh_index, aiMesh* mesh_node)
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

    mesh_t mesh = gl_create_mesh_array(&vb[0], &ib[0], (uint32)vb.size(), (uint32)ib.size());
    meshgroup.meshes[mesh_index] = mesh;
    meshgroup.mesh_to_texture[mesh_index] = mesh_node->mMaterialIndex;
}

internal void assimp_load_meshgroup(meshgroup_t& meshgroup, const char* file_name)
{
    win64_global_timestamp();

    Assimp::Importer importer;
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
    console_printf("took %f seconds to Importer::ReadFile\n", win64_global_timestamp());

    meshgroup.meshes.resize(scene->mNumMeshes);
    meshgroup.mesh_to_texture.resize(scene->mNumMeshes);
    meshgroup.textures.resize(scene->mNumMaterials);

    //console_printf("took %f seconds to resize 3 vectors\n", win64_global_timestamp());

    // Unpack meshes
    for(size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        __assimp_load_mesh_helper(meshgroup, i, scene->mMeshes[i]);
    }

    console_printf("took %f seconds to unpack all the meshes\n", win64_global_timestamp());

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
                gl_load_texture_from_file(meshgroup.textures[i], tex_path.c_str());
            }
        }
    }

    console_printf("took %f seconds to load all the textures\n", win64_global_timestamp());
}