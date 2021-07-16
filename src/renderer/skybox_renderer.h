#pragma once

#include "texture.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"

struct skybox_renderer
{
    void init();

    void render(const camera_t& camera);

    cubemap_t   skybox_cubemap;

private:
    mesh_t      skybox_mesh;

    shader_t    skybox_shader;

    void load_shader();
};
