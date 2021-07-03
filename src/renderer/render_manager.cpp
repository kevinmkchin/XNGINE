#include "render_manager.h"
#include <GL/glew.h>
#include "material.h"
#include "../runtime/game_state.h"
#include "../stb/stb_sprintf.h"
#include "../debugging/console.h"
#include "../debugging/profiling/profiler.h"
#include "../debugging/debug_drawer.h"
#include "../core/input.h"

SINGLETON_INIT(render_manager)

static const char* vertex_shader_path = "shaders/default_phong.vert";
static const char* frag_shader_path = "shaders/default_phong.frag";
static const char* deferred_geometry_vs_path = "shaders/deferred_geometry_pass.vert";
static const char* deferred_geometry_fs_path = "shaders/deferred_geometry_pass.frag";
static const char* deferred_lighting_vs_path = "shaders/deferred_lighting_pass.vert";
static const char* deferred_lighting_fs_path = "shaders/deferred_lighting_pass.frag";
static const char* ui_vs_path = "shaders/ui.vert";
static const char* ui_fs_path = "shaders/ui.frag";
static const char* text_vs_path = "shaders/text_ui.vert";
static const char* text_fs_path = "shaders/text_ui.frag";
static const char* simple_vs_path = "shaders/simple.vert";
static const char* simple_fs_path = "shaders/simple.frag";

// Temporary
bool g_b_wireframe = false;

material_t material_shiny = {4.f, 128.f };
material_t material_dull = {0.5f, 1.f };


void render_manager::initialize()
{
    // Initialize GLEW
    glewExperimental = GL_TRUE; // Enable us to access modern opengl extension features
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW failed to initialize.\n");
        // todo return false;
    }
    console_printf("GLEW initialized.\n");

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // alpha blending func: a * (rgb) + (1 - a) * (rgb) = final color output
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_CULL_FACE);

    update_buffer_size(back_buffer_width, back_buffer_height);
    matrix_projection_ortho = projection_matrix_orthographic_2d(0.0f, (float)back_buffer_width, (float)back_buffer_height, 0.0f);
}

void render_manager::render()
{
    //render_pass_directional_shadow_map();
    //render_pass_omnidirectional_shadow_map();
    render_pass_main();
}

void render_manager::render_pass_directional_shadow_map()
{
    shader_t::gl_use_shader(shader_directional_shadow_map);

    shader_directional_shadow_map.gl_bind_matrix4fv("directionalLightTransform", 1, directional_shadow_map.directionalLightSpaceMatrix.ptr());
    glViewport(0, 0, directional_shadow_map.SHADOW_WIDTH, directional_shadow_map.SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, directional_shadow_map.directionalShadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    //glCullFace(GL_FRONT);

    render_scene(shader_directional_shadow_map);

    //glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_manager::render_pass_omnidirectional_shadow_map()
{
    temp_map_t& loaded_map = gs->loaded_map;

    shader_t::gl_use_shader(shader_omni_shadow_map);
    for(int omniLightCount = 0; omniLightCount < loaded_map.pointlights.size() + loaded_map.spotlights.size(); ++omniLightCount)
    {
        glViewport(0, 0, omni_shadow_maps[omniLightCount].CUBE_SHADOW_WIDTH, omni_shadow_maps[omniLightCount].CUBE_SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, omni_shadow_maps[omniLightCount].depthCubeMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        shader_omni_shadow_map.gl_bind_matrix4fv("lightMatrices[0]", 6, (float*) omni_shadow_maps[omniLightCount].shadowTransforms.data());
        vec3 lightPos = loaded_map.pointlights[omniLightCount].position;
        shader_omni_shadow_map.gl_bind_3f("lightPos", lightPos.x, lightPos.y, lightPos.z);
        shader_omni_shadow_map.gl_bind_1f("farPlane", omni_shadow_maps[omniLightCount].depthCubeMapFarPlane);

        render_scene(shader_omni_shadow_map);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void render_manager::render_pass_main()
{
    camera_t& camera = gs->m_camera;
    temp_map_t& loaded_map = gs->loaded_map;

    glViewport(0, 0, back_buffer_width, back_buffer_height);
    //glClearColor(0.39f, 0.582f, 0.926f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear opengl context's buffer

// NOT ALPHA BLENDED
    glDisable(GL_BLEND);
// DEPTH TESTED
    glEnable(GL_DEPTH_TEST);
    // TODO Probably should make own shader for wireframe draws so that wireframe fragments aren't affected by lighting or textures
    if (g_b_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    camera.calculate_view_matrix();

/////////////////////////////////////////////////////////////
// 1. Geometry pass
/////////////////////////////////////////////////////////////
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_FBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader_t::gl_use_shader(shader_deferred_geometry_pass);

    shader_deferred_geometry_pass.gl_bind_matrix4fv("matrix_view", 1, camera.matrix_view.ptr());
    shader_deferred_geometry_pass.gl_bind_matrix4fv("matrix_proj_perspective", 1, camera.matrix_perspective.ptr());

    shader_deferred_geometry_pass.gl_bind_1i("texture_sampler_0", 1);

    render_scene(shader_deferred_geometry_pass);

glBindFramebuffer(GL_FRAMEBUFFER, 0);

/////////////////////////////////////////////////////////////
// 2. Lighting pass
/////////////////////////////////////////////////////////////
    shader_t::gl_use_shader(shader_deferred_lighting_pass);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position_texture);
    shader_deferred_lighting_pass.gl_bind_1i("gPosition", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal_texture);
    shader_deferred_lighting_pass.gl_bind_1i("gNormal", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_albedo_texture);
    shader_deferred_lighting_pass.gl_bind_1i("gAlbedo", 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, g_specular_shininess_texture);
    shader_deferred_lighting_pass.gl_bind_1i("gSpecularAndShininess", 3);

    shader_deferred_lighting_pass.gl_bind_3f("observer_pos", camera.position.x, camera.position.y, camera.position.z);

    {
        directional_light_t light = loaded_map.directionallight;
        shader_deferred_lighting_pass.gl_bind_3f("directional_light.colour", light.colour.x, light.colour.y, light.colour.z);
        shader_deferred_lighting_pass.gl_bind_1f("directional_light.ambient_intensity", light.ambient_intensity);
        shader_deferred_lighting_pass.gl_bind_1f("directional_light.diffuse_intensity", light.diffuse_intensity);
        vec3 direction = orientation_to_direction(light.orientation);
        shader_deferred_lighting_pass.gl_bind_3f("directional_light.direction", direction.x, direction.y, direction.z);

//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, directional_shadow_map.directionalShadowMapTexture);
//        shader_common.gl_bind_1i("directionalShadowMap", 2);
//        shader_common.gl_bind_matrix4fv("directionalLightTransform", 1, directional_shadow_map.directionalLightSpaceMatrix.ptr());
    }

    {
//        for(int omniLightCount = 0; omniLightCount < loaded_map.pointlights.size() + loaded_map.spotlights.size(); ++omniLightCount)
//        {
//            glActiveTexture(GL_TEXTURE3 + omniLightCount);
//            glBindTexture(GL_TEXTURE_CUBE_MAP, omni_shadow_maps[omniLightCount].depthCubeMapTexture);
//
//            char name_buffer[128] = {'\0'};
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "omniShadowMaps[%d].shadowMap", omniLightCount);
//            shader_common.gl_bind_1i(name_buffer, 3 + omniLightCount);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "omniShadowMaps[%d].farPlane", omniLightCount);
//            shader_common.gl_bind_1f(name_buffer, omni_shadow_maps[omniLightCount].depthCubeMapFarPlane);
//        }

        std::vector<point_light_t> plights = loaded_map.pointlights;
        shader_deferred_lighting_pass.gl_bind_1i("point_light_count", plights.size());
        for(size_t i = 0; i < plights.size(); ++i)
        {
            char name_buffer[128] = {'\0'};
            stbsp_snprintf(name_buffer, sizeof(name_buffer), "point_light[%d].colour", i);
            shader_deferred_lighting_pass.gl_bind_3f(name_buffer, plights[i].colour.x, plights[i].colour.y, plights[i].colour.z);
            stbsp_snprintf(name_buffer, sizeof(name_buffer), "point_light[%d].ambient_intensity", i);
            shader_deferred_lighting_pass.gl_bind_1f(name_buffer, plights[i].ambient_intensity);
            stbsp_snprintf(name_buffer, sizeof(name_buffer), "point_light[%d].diffuse_intensity", i);
            shader_deferred_lighting_pass.gl_bind_1f(name_buffer, plights[i].diffuse_intensity);
            stbsp_snprintf(name_buffer, sizeof(name_buffer), "point_light[%d].position", i);
            shader_deferred_lighting_pass.gl_bind_3f(name_buffer, plights[i].position.x, plights[i].position.y, plights[i].position.z);
            stbsp_snprintf(name_buffer, sizeof(name_buffer), "point_light[%d].att_constant", i);
            shader_deferred_lighting_pass.gl_bind_1f(name_buffer, plights[i].att_constant);
            stbsp_snprintf(name_buffer, sizeof(name_buffer), "point_light[%d].att_linear", i);
            shader_deferred_lighting_pass.gl_bind_1f(name_buffer, plights[i].att_linear);
            stbsp_snprintf(name_buffer, sizeof(name_buffer), "point_light[%d].att_quadratic", i);
            shader_deferred_lighting_pass.gl_bind_1f(name_buffer, plights[i].att_quadratic);
        }

//        std::vector<spot_light_t> slights = loaded_map.spotlights;
//        shader_common.gl_bind_1i("spot_light_count", slights.size());
//        for(size_t i = 0; i < slights.size(); ++i)
//        {
//            char name_buffer[128] = {'\0'};
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].plight.colour", i);
//            shader_common.gl_bind_3f(name_buffer, slights[i].colour.x, slights[i].colour.y, slights[i].colour.z);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].plight.ambient_intensity", i);
//            shader_common.gl_bind_1f(name_buffer, slights[i].ambient_intensity);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].plight.diffuse_intensity", i);
//            shader_common.gl_bind_1f(name_buffer, slights[i].diffuse_intensity);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].plight.position", i);
//            shader_common.gl_bind_3f(name_buffer, slights[i].position.x, slights[i].position.y, slights[i].position.z);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].plight.att_constant", i);
//            shader_common.gl_bind_1f(name_buffer, slights[i].att_constant);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].plight.att_linear", i);
//            shader_common.gl_bind_1f(name_buffer, slights[i].att_linear);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].plight.att_quadratic", i);
//            shader_common.gl_bind_1f(name_buffer, slights[i].att_quadratic);
//
//            vec3 direction = orientation_to_direction(slights[i].orientation);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].direction", i);
//            shader_common.gl_bind_3f(name_buffer, direction.x, direction.y, direction.z);
//            stbsp_snprintf(name_buffer, sizeof(name_buffer), "spot_light[%d].cutoff", i);
//            shader_common.gl_bind_1f(name_buffer, slights[i].cosine_cutoff());
//        }
    }

    {
        local_persist mesh_t quad;
        local_persist bool meshmade = false;
        if (!meshmade) {
            meshmade = true;
            u32 quadindices[6] = {
                    0, 1, 3,
                    0, 3, 2
            };
            GLfloat quadvertices[16] = {
                    //  x   y    u    v
                    -1.f, -1.f, 0.f, 0.f,
                    1.f, -1.f, 1.f, 0.f,
                    -1.f, 1.f, 0.f, 1.f,
                    1.f, 1.f, 1.f, 1.f
            };
            mesh_t::gl_create_mesh(quad, quadvertices, quadindices, 16, 6, 2, 2, 0);
        }
        quad.gl_render_mesh();
    }

    glUseProgram(0);

// ALPHA BLENDED
    glEnable(GL_BLEND);
    // todo this is being hidden behind deferred rendered quad    debug_render(shader_simple, camera);

// NOT DEPTH TESTED
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    debug_render(shader_simple, camera);
//#if INTERNAL_BUILD
//    if(input::get_instance()->g_keystate[SDL_SCANCODE_F3])
//    {
//        local_persist mesh_t quad;
//        local_persist bool meshmade = false;
//        if(!meshmade)
//        {
//            meshmade = true;
//
//            u32 quadindices[6] = {
//                    0, 1, 3,
//                    0, 3, 2
//            };
//            GLfloat quadvertices[16] = {
//                    //  x     y        u    v
//                    -1.f, -1.f,   -0.1f, -0.1f,
//                    1.f, -1.f,    1.1f, -0.1f,
//                    -1.f, 1.f,    -0.1f, 1.1f,
//                    1.f, 1.f,     1.1f, 1.1f
//            };
//            mesh_t::gl_create_mesh(quad, quadvertices, quadindices, 16, 6, 2, 2, 0);
//        }
//        shader_t::gl_use_shader(shader_debug_dir_shadow_map);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, directional_shadow_map.directionalShadowMapTexture);
//        quad.gl_render_mesh();
//        glUseProgram(0);
//    }
//#endif

    profiler_render(&shader_ui, &shader_text);
    console_render(&shader_ui, &shader_text);

    // Enable depth test before swapping buffers
    // (NOTE: if we don't enable depth test before swap, the shadow map shows up as blank white texture on the quad.)
    glEnable(GL_DEPTH_TEST);
}

void render_manager::render_scene(shader_t& shader)
{
    temp_map_t& loaded_map = gs->loaded_map;

    /** We could simply update the game object's position, rotation, scale fields,
        then construct the model matrix in game_render based on those fields.
    */
    mat4 matrix_model = identity_mat4();
    matrix_model = identity_mat4();
    matrix_model *= translation_matrix(loaded_map.mainobject.pos);
    matrix_model *= rotation_matrix(loaded_map.mainobject.orient);
    matrix_model *= scale_matrix(loaded_map.mainobject.scale);
    shader.gl_bind_matrix4fv("matrix_model", 1, matrix_model.ptr());
    if(shader.get_cached_uniform_location("material.specular_intensity") >= 0)
    {
        shader.gl_bind_1f("material.specular_intensity", material_dull.specular_intensity);
        shader.gl_bind_1f("material.shininess", material_dull.shininess);
    }
    loaded_map.mainobject.model.render();
}

void
render_manager::load_shaders()
{
    shader_t::gl_load_shader_program_from_file(shader_common, vertex_shader_path, frag_shader_path);
    shader_t::gl_load_shader_program_from_file(shader_deferred_geometry_pass, deferred_geometry_vs_path, deferred_geometry_fs_path);
    shader_t::gl_load_shader_program_from_file(shader_deferred_lighting_pass, deferred_lighting_vs_path, deferred_lighting_fs_path);
    shader_t::gl_load_shader_program_from_file(shader_directional_shadow_map, "shaders/directional_shadow_map.vert", "shaders/directional_shadow_map.frag");
    shader_t::gl_load_shader_program_from_file(shader_omni_shadow_map, "shaders/omni_shadow_map.vert", "shaders/omni_shadow_map.geom", "shaders/omni_shadow_map.frag");
    shader_t::gl_load_shader_program_from_file(shader_debug_dir_shadow_map, "shaders/debug_directional_shadow_map.vert", "shaders/debug_directional_shadow_map.frag");
    shader_t::gl_load_shader_program_from_file(shader_text, text_vs_path, text_fs_path);
    shader_t::gl_load_shader_program_from_file(shader_ui, ui_vs_path, ui_fs_path);
    shader_t::gl_load_shader_program_from_file(shader_simple, simple_vs_path, simple_fs_path);
}

void render_manager::clean_up()
{
    shader_t::gl_delete_shader(shader_common);
    shader_t::gl_delete_shader(shader_text);
    shader_t::gl_delete_shader(shader_ui);
}

vec2i render_manager::get_buffer_size()
{
    vec2i retval = {back_buffer_width, back_buffer_height };
    return retval;
}

void render_manager::update_buffer_size(i32 new_width, i32 new_height)
{
    back_buffer_width = new_width;
    back_buffer_height = new_height;
    glViewport(0, 0, back_buffer_width, back_buffer_height);

    console_printf("Viewport updated - x: %d y: %d\n", back_buffer_width, back_buffer_height);
}

void render_manager::temp_create_shadow_maps()
{
    temp_map_t& loaded_map = gs->loaded_map;

    glGenFramebuffers(1, &directional_shadow_map.directionalShadowMapFBO);

    glGenTextures(1, &directional_shadow_map.directionalShadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, directional_shadow_map.directionalShadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 directional_shadow_map.SHADOW_WIDTH, directional_shadow_map.SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float smap_bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, smap_bordercolor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, directional_shadow_map.directionalShadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, directional_shadow_map.directionalShadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mat4 lightProjection = projection_matrix_orthographic(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 150.f);
    directional_shadow_map.directionalLightSpaceMatrix = lightProjection
            //* view_matrix_look_at(-orientation_to_direction(loaded_maps[0].directionallight.orientation) + make_vec3(-47.f, 66.f, 0.f), make_vec3(-47.f, 66.f, 0.f), make_vec3(0.f,1.f,0.f)); // TODO make up 0,0,1 if light is straight up or down
            //* view_matrix_look_at(make_vec3(-2.0f, 4.0f, -1.0f), make_vec3(0.f, 0.f, 0.f), make_vec3(0.f,1.f,0.f)); // TODO make up 0,0,1 if light is straight up or down
            * view_matrix_look_at(make_vec3(-47.44f, 66.29f, 9.65f), make_vec3(-47.44f, 66.29f, 9.65f) + orientation_to_direction(loaded_map.directionallight.orientation), make_vec3(0.f,1.f,0.f)); // TODO make up 0,0,1 if light is straight up or down

    for(int omniLightCount = 0; omniLightCount < loaded_map.pointlights.size() + loaded_map.spotlights.size(); ++omniLightCount)
    {
        glGenFramebuffers(1, &omni_shadow_maps[omniLightCount].depthCubeMapFBO);

        glGenTextures(1, &omni_shadow_maps[omniLightCount].depthCubeMapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, omni_shadow_maps[omniLightCount].depthCubeMapTexture);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         omni_shadow_maps[omniLightCount].CUBE_SHADOW_WIDTH, omni_shadow_maps[omniLightCount].CUBE_SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, omni_shadow_maps[omniLightCount].depthCubeMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, omni_shadow_maps[omniLightCount].depthCubeMapTexture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        float aspect = (float)omni_shadow_maps[omniLightCount].CUBE_SHADOW_WIDTH/(float)omni_shadow_maps[omniLightCount].CUBE_SHADOW_HEIGHT;
        float nearPlane = 1.0f;
        mat4 shadowProj = projection_matrix_perspective(90.f * KC_DEG2RAD, aspect, nearPlane, omni_shadow_maps[omniLightCount].depthCubeMapFarPlane);

        vec3 lightPos = loaded_map.pointlights[omniLightCount].position;
        omni_shadow_maps[omniLightCount].shadowTransforms.push_back(
                shadowProj * view_matrix_look_at(lightPos, lightPos + WORLD_FORWARD_VECTOR, WORLD_DOWN_VECTOR));
        omni_shadow_maps[omniLightCount].shadowTransforms.push_back(
                shadowProj * view_matrix_look_at(lightPos, lightPos + WORLD_BACKWARD_VECTOR, WORLD_DOWN_VECTOR));
        omni_shadow_maps[omniLightCount].shadowTransforms.push_back(
                shadowProj * view_matrix_look_at(lightPos, lightPos + WORLD_UP_VECTOR, WORLD_RIGHT_VECTOR));
        omni_shadow_maps[omniLightCount].shadowTransforms.push_back(
                shadowProj * view_matrix_look_at(lightPos, lightPos + WORLD_DOWN_VECTOR, WORLD_LEFT_VECTOR));
        omni_shadow_maps[omniLightCount].shadowTransforms.push_back(
                shadowProj * view_matrix_look_at(lightPos, lightPos + WORLD_RIGHT_VECTOR, WORLD_DOWN_VECTOR));
        omni_shadow_maps[omniLightCount].shadowTransforms.push_back(
                shadowProj * view_matrix_look_at(lightPos, lightPos + WORLD_LEFT_VECTOR, WORLD_DOWN_VECTOR));
    }
}

void render_manager::temp_create_geometry_buffer()
{
    // todo regenerate buffers when screen size change
    glGenFramebuffers(1, &g_buffer_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_FBO);

    glGenTextures(1, &g_position_texture);
    glBindTexture(GL_TEXTURE_2D, g_position_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, back_buffer_width, back_buffer_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position_texture, 0);

    glGenTextures(1, &g_normal_texture);
    glBindTexture(GL_TEXTURE_2D, g_normal_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, back_buffer_width, back_buffer_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal_texture, 0);

    glGenTextures(1, &g_albedo_texture);
    glBindTexture(GL_TEXTURE_2D, g_albedo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, back_buffer_width, back_buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_albedo_texture, 0);

    glGenTextures(1, &g_specular_shininess_texture);
    glBindTexture(GL_TEXTURE_2D, g_specular_shininess_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, back_buffer_width, back_buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_specular_shininess_texture, 0);

    u32 color_attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, color_attachments);

    glGenRenderbuffers(1, &g_depth_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, g_depth_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, back_buffer_width, back_buffer_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_depth_RBO);
}
