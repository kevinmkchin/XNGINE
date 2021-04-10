INTERNAL void render_mesh_group(MeshGroup& mesh_group)
{
    for(mi i = 0; i < mesh_group.meshes.size(); ++i)
    {
        uint16 mat_index = mesh_group.mesh_to_texture[i];
        if(mat_index < mesh_group.textures.size() && mesh_group.textures[mat_index].texture_id != 0)
        {
            gl_use_texture(mesh_group.textures[mat_index]);
        }

        gl_render_mesh(mesh_group.meshes[i]);
    }
}

INTERNAL void clear_mesh_group(MeshGroup& mesh_group)
{
    for(mi i = 0; i < mesh_group.meshes.size(); ++i)
    {
        gl_delete_mesh(mesh_group.meshes[i]);
    }
    for(mi i = 0; i < mesh_group.textures.size(); ++i)
    {
        gl_delete_texture(mesh_group.textures[i]);
    }
}

INTERNAL void assimp_load_mesh(MeshGroup& mesh_group, aiMesh* mesh_node, const aiScene* scene)
{
    // CHECK TIME ELAPSED WITH AND WITHOUT void resize (size_type n, value_type val = value_type());
    std::vector<real32> vb;
    std::vector<uint32> ib;
    for(mi i = 0; i < mesh_node->mNumVertices; ++i)
    {
        vb.insert(vb.end(), { mesh_node->mVertices[i].x, mesh_node->mVertices[i].y, mesh_node->mVertices[i].z });
        if(mesh_node->mTextureCoords[0])
        {
            vb.insert(vb.end(), { mesh_node->mTextureCoords[0][i].x, mesh_node->mTextureCoords[0][i].y });
        }
        else
        {
            vb.insert(vb.end(), { 0.f, 0.f });
        }
        vb.insert(vb.end(), { mesh_node->mNormals[i].x, mesh_node->mNormals[i].y, mesh_node->mNormals[i].z });
    }

    for(mi i = 0; i < mesh_node->mNumFaces; ++i)
    {
        aiFace face = mesh_node->mFaces[i];
        for(mi j = 0; j < face.mNumIndices; ++j)
        {
            ib.push_back(face.mIndices[j]);
        }
    }

    Mesh mesh = gl_create_mesh_array(&vb[0], &ib[0], (uint32)vb.size(), (uint32)ib.size());
    mesh_group.meshes.push_back(mesh);
    mesh_group.mesh_to_texture.push_back(mesh_node->mMaterialIndex);
}

INTERNAL void assimp_load_node(MeshGroup& mesh_group, aiNode* node, const aiScene* scene)
{
    for(mi i = 0; i < node->mNumMeshes; ++i)
    {
        assimp_load_mesh(mesh_group, scene->mMeshes[node->mMeshes[i]], scene);
    }

    for(mi i = 0; i < node->mNumChildren; ++i)
    {
        assimp_load_node(mesh_group, node->mChildren[i], scene);
    }
}

INTERNAL void assimp_load_materials(MeshGroup& mesh_group, const aiScene* scene)
{
    mesh_group.textures.resize(scene->mNumMaterials);
    
    for(mi i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* mat = scene->mMaterials[i];
        if(mat->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString path;
            if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                int idx = (int)std::string(path.data).rfind("\\");
                std::string filename = std::string(path.data).substr(idx+1);

                std::string tex_path = std::string("data/textures/") + filename;
                gl_load_texture_from_file(mesh_group.textures[i], tex_path.c_str());
            }
        }
    }
}

INTERNAL void assimp_load_mesh_group(MeshGroup& mesh_group, const char* file_name)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_name, aiProcess_Triangulate
        |aiProcess_GenNormals|aiProcess_JoinIdenticalVertices); //|aiProcess_FlipUVs|aiProcess_GenSmoothNormals
    if(!scene)
    {
        con_printf("Model '%s' failed to load: %s\n", file_name, importer.GetErrorString());
        return;
    }

    assimp_load_node(mesh_group, scene->mRootNode, scene);
    assimp_load_materials(mesh_group, scene);
}