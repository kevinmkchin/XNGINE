#pragma once

#include "../gamedefine.h"
#include "shader.h"
#include "../core/kc_math.h"

struct game_state;

struct directional_shadow_map_t
{
    const i32 SHADOW_WIDTH = 2048;
    const i32 SHADOW_HEIGHT = 2048;
    u32 directionalShadowMapTexture = 0;
    u32 directionalShadowMapFBO = 0;
    mat4 directionalLightSpaceMatrix;
};

struct omni_shadow_map_t
{
    const i32 CUBE_SHADOW_WIDTH = 1024;
    const i32 CUBE_SHADOW_HEIGHT = 1024;
    u32 depthCubeMapTexture = 0;
    u32 depthCubeMapFBO = 0;
    float depthCubeMapFarPlane = 25.f;
    std::vector<mat4> shadowTransforms;
};

struct display_settings_t
{
    //todo
};

struct render_manager
{
    void initialize();

    void render();

    void load_shaders();

    void clean_up();

    vec2i get_buffer_size();

    void update_buffer_size(i32 new_width, i32 new_height);

    void temp_create_shadow_maps();

    void temp_create_geometry_buffer();

    game_state* gs = nullptr;

    mat4 matrix_projection_ortho;

private:

    void render_pass_directional_shadow_map();

    void render_pass_omnidirectional_shadow_map();

    void render_pass_main();

    void render_scene(shader_t& shader);

    // Width and Height of writable buffer
    i32 back_buffer_width = -1;
    i32 back_buffer_height = -1;

    shader_t    shader_common;
    shader_t    shader_deferred_geometry_pass;
    shader_t    shader_deferred_lighting_pass;
    shader_t    shader_directional_shadow_map;
    shader_t    shader_omni_shadow_map;
    shader_t    shader_debug_dir_shadow_map;
    shader_t    shader_text;
    shader_t    shader_ui;
    shader_t    shader_simple;

    directional_shadow_map_t directional_shadow_map;
    omni_shadow_map_t omni_shadow_maps[42];

    u32 g_buffer_FBO = 0;
    u32 g_position_texture = 0;
    u32 g_normal_texture = 0;
    u32 g_albedo_texture = 0;
    u32 g_specular_shininess_texture = 0;
    u32 g_depth_RBO = 0;

    SINGLETON(render_manager)
};