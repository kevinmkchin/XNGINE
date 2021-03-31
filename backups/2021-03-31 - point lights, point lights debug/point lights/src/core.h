#pragma once

/**

    Types to hold data in memory

*/

/** Handle for a file in memory */
struct BinaryFileHandle 
{
    uint64  size    = 0;    // size of file in memory
    void*   memory  = NULL; // pointer to file in memory
};

/** Handle for an UNSIGNED BYTE bitmap in memory */
struct BitmapHandle : BinaryFileHandle
{
    // uint64 size
    // void* memory
    uint32  width = 0;      // image width
    uint32  height = 0;     // image height
    uint8   bit_depth = 0;  // bit depth of bitmap in bytes (e.g. bit depth = 3 means there are 3 bytes in the bitmap per pixel)
};

/** 

    Core game engine objects

*/

/** Handle for Shader Program stored in GPU memory */
struct BaseShader
{
    GLuint  id_shader_program = 0;    // id of this shader program in GPU memory

    GLint   id_uniform_model = 0;    // location id for the model matrix uniform variable

    GLint uniform_location(const char* uniform_name)
    {
        GLint location = glGetUniformLocation(id_shader_program, uniform_name);
        if (location == 0xffffffff) {
            con_printf("Warning! Unable to get the location of uniform '%s' for shader id %d...\n", uniform_name, id_shader_program);
        }
        return location;
    }

    virtual void load_uniforms()
    {
        id_uniform_model = uniform_location("matrix_model");
    }
};

struct PerspectiveShader : BaseShader
{
    GLint id_uniform_proj_perspective = 0; // location id for the perspective projection matrix
    GLint id_uniform_view = 0;    // location id for the view matrix uniform variable

    virtual void load_uniforms() override
    {
        BaseShader::load_uniforms();
        id_uniform_proj_perspective = uniform_location("matrix_proj_perspective");
        id_uniform_view = uniform_location("matrix_view");
    }
};

struct OrthographicShader : BaseShader
{
    GLint id_uniform_proj_orthographic = 0; // location id for the perspective projection matrix

    virtual void load_uniforms() override
    {
        BaseShader::load_uniforms();
        id_uniform_proj_orthographic = uniform_location("matrix_proj_orthographic");
    }
};

struct LightingShader : PerspectiveShader
{
    GLOBAL_VAR const unsigned int MAX_POINT_LIGHTS = 4;
    GLOBAL_VAR const unsigned int MAX_SPOT_LIGHTS = 2;

    GLint   id_uniform_observer_pos = 0;

    struct {
        GLint colour;
        GLint ambient_intensity;
        GLint diffuse_intensity;

        GLint direction;
    } id_uniform_directional_light;

    GLint   id_uniform_point_light_count = 0;
    struct {
        GLint colour;
        GLint ambient_intensity;
        GLint diffuse_intensity;

        GLint position;
        GLint att_constant;
        GLint att_linear;
        GLint att_quadratic;
    } id_uniform_point_light[MAX_POINT_LIGHTS];

    GLint   id_uniform_specular_intensity = 0;
    GLint   id_uniform_shininess = 0;

    virtual void load_uniforms() override
    {
        PerspectiveShader::load_uniforms();
        id_uniform_observer_pos = uniform_location("observer_pos");
        id_uniform_directional_light.colour = uniform_location("directional_light.colour");
        id_uniform_directional_light.ambient_intensity = uniform_location("directional_light.ambient_intensity");
        id_uniform_directional_light.diffuse_intensity = uniform_location("directional_light.diffuse_intensity");
        id_uniform_directional_light.direction = uniform_location("directional_light.direction");
        id_uniform_point_light_count = uniform_location("point_light_count");
        for(mi i = 0; i < MAX_POINT_LIGHTS; ++i)
        {
            char loc_buffer[128] = {'\0'};
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "point_light[%d].colour", i);
            id_uniform_point_light[i].colour = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "point_light[%d].ambient_intensity", i);
            id_uniform_point_light[i].ambient_intensity = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "point_light[%d].diffuse_intensity", i);
            id_uniform_point_light[i].diffuse_intensity = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "point_light[%d].position", i);
            id_uniform_point_light[i].position = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "point_light[%d].att_constant", i);
            id_uniform_point_light[i].att_constant = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "point_light[%d].att_linear", i);
            id_uniform_point_light[i].att_linear = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "point_light[%d].att_quadratic", i);
            id_uniform_point_light[i].att_quadratic = uniform_location(loc_buffer);
        }
        id_uniform_specular_intensity = uniform_location("material.specular_intensity");
        id_uniform_shininess = uniform_location("material.shininess");
    }
};

struct Material
{
    real32 specular_intensity = 0.f;
    real32 shininess = 1.f;
};

struct Light
{
    glm::vec3   colour              = glm::vec3(1.f, 1.f, 1.f);
    real32      ambient_intensity   = 0.2f;
    real32      diffuse_intensity   = 1.0f;  
};

struct DirectionalLight : Light
{
    glm::vec3   direction           = glm::vec3(0.f, -1.f, 0.f);
};

struct PointLight : Light
{
    glm::vec3   position = glm::vec3(0.f, 0.f, 0.f);
    // Attenuation coefficients
    GLfloat     att_constant = 1.0f;
    GLfloat     att_linear = 0.f;
    GLfloat     att_quadratic = 0.f;
};

/*
struct ShaderGroup
{
    CommonShader
    TextShader
    UIShader
};
*/

/** Stores mesh { VAO, VBO, IBO } info. Handle for VAO on GPU memory */
struct Mesh
{
    // Holds the ID for the VAO, VBO, IBO in the GPU memory
    uint32  id_vao          = 0;
    uint32  id_vbo          = 0;
    uint32  id_ibo          = 0;
    int32   index_count     = 0;
};

/** Handle for Texture stored in GPU memory */
struct Texture
{
    GLuint      texture_id  = 0;        // ID for the texture in GPU memory
    int32       width       = 0;        // Width of the texture
    int32       height      = 0;        // Height of the texture
    GLenum      format      = GL_NONE;  // format / bitdepth of texture (GL_RGB would be 3 byte bit depth)
};

/** Camera properties */
struct Camera
{
    glm::vec3   position                = glm::vec3(0.f);           // camera x y z pos in world space 
    glm::vec3   rotation                = glm::vec3(0.f);           // pitch, yaw, roll - in that order
    glm::vec3   world_up                = glm::vec3(0.f, 1.f, 0.f);

    glm::vec3   calculated_direction    = glm::vec3(0.f);           // Intuitive direction - direction forward
    glm::vec3   calculated_up           = glm::vec3(0.f);
    glm::vec3   calculated_right        = glm::vec3(0.f);

    real32      movespeed               = 2.f;
    real32      turnspeed               = 0.17f;

    glm::mat4   matrix_perspective; // Perspective projection matrix
    glm::mat4   matrix_view;        // Last calculated view matrix
};

/**  */
/* 
struct Entity
{
    glm::vec3   pos     = glm::vec3(0.f);
    glm::vec3   rot     = glm::vec3(0.f);
    glm::vec3   scale   = glm::vec3(0.f);
    Mesh        mesh;
    // Collider col
    // boolean flags
    // bool b_act; // active flag
    // bool b_col; // collidable flag
    // Tags tags[4]; // primary, secondary, tertiary, quaternary tags
};
*/

/*
// Data-oriented example:
// enemy id is the index in the array
struct Enemies
{
    vec3* pos;
    uint16 pos_count;
    vec3* rot;
    collider* cols;
    uint16 col_count;
};
*/