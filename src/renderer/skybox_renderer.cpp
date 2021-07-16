#include "skybox_renderer.h"
#include "../core/kc_math.h"

internal u32 skybox_indices[] = {
        // front
        0, 1, 2,
        2, 1, 3,
        // right
        2, 3, 5,
        5, 3, 7,
        // back
        5, 7, 4,
        4, 7, 6,
        // left
        4, 6, 0,
        0, 6, 1,
        // top
        4, 0, 5,
        5, 0, 2,
        // bottom
        1, 6, 3,
        3, 6, 7
};

internal float skybox_vertices[] = {
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
};

void skybox_renderer::init()
{
    mesh_t::gl_create_mesh(skybox_mesh, skybox_vertices, skybox_indices, array_count(skybox_vertices),
                           array_count(skybox_indices), 3, 0, 0);

    load_shader();
}

void skybox_renderer::render(const camera_t& camera)
{
    shader_t::gl_use_shader(skybox_shader);

    mat4 skybox_view_matrix = make_mat4(make_mat3(camera.matrix_view));
    skybox_shader.gl_bind_matrix4fv("matrix_view", 1, skybox_view_matrix.ptr());
    skybox_shader.gl_bind_matrix4fv("matrix_proj_perspective", 1, camera.matrix_perspective.ptr());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap.texture_id);

    skybox_mesh.gl_render_mesh();

    glUseProgram(0);
}

void skybox_renderer::load_shader()
{
    shader_t::gl_load_shader_program_from_file(skybox_shader, "shaders/skybox.vert", "shaders/skybox.frag");
}

