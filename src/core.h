#pragma once

/**

    Types to hold data in memory

*/

/** Handle for a file in memory */
struct binary_file_handle_t
{
    uint64  size    = 0;    // size of file in memory
    void*   memory  = NULL; // pointer to file in memory
};

/** Handle for an UNSIGNED BYTE bitmap in memory */
struct bitmap_handle_t : binary_file_handle_t
{
    uint32  width = 0;      // image width
    uint32  height = 0;     // image height
    uint8   bit_depth = 0;  // bit depth of bitmap in bytes (e.g. bit depth = 3 means there are 3 bytes in the bitmap per pixel)
};

/** 

    Core game engine objects
    
    Rules:
     - unit vector (x: 1, y: 0, z: 0), aka positive X, is the "forward" direction for objects
     - direction != orientation
     - orientation ~= rotation
     - vec3(1, 0, 0) rotated by the object's orientation (represented by quaternion) is the object's forward direction
     - if quaternion is used to represent an orientation, then the quaternion represents the rotation from the WORLD FORWARD VECTOR
     - RIGHT HAND RULE for everything
*/

/** Handle for Shader Program stored in GPU memory */
struct shader_base_t
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

struct shader_perspective_t : shader_base_t
{
    GLint id_uniform_proj_perspective = 0; // location id for the perspective projection matrix
    GLint id_uniform_view = 0;    // location id for the view matrix uniform variable

    virtual void load_uniforms() override
    {
        shader_base_t::load_uniforms();
        id_uniform_proj_perspective = uniform_location("matrix_proj_perspective");
        id_uniform_view = uniform_location("matrix_view");
    }
};

struct shader_orthographic_t : shader_base_t
{
    GLint id_uniform_proj_orthographic = 0; // location id for the perspective projection matrix

    virtual void load_uniforms() override
    {
        shader_base_t::load_uniforms();
        id_uniform_proj_orthographic = uniform_location("matrix_proj_orthographic");
    }
};

struct shader_lighting_t : shader_perspective_t
{
    global_var const unsigned int MAX_POINT_LIGHTS = 4;
    global_var const unsigned int MAX_SPOT_LIGHTS = 4;

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

    GLint   id_uniform_spot_light_count = 0;
    struct {
        GLint colour;
        GLint ambient_intensity;
        GLint diffuse_intensity;

        GLint position;
        GLint att_constant;
        GLint att_linear;
        GLint att_quadratic;

        GLint direction;
        GLint cos_cutoff;
    } id_uniform_spot_light[MAX_SPOT_LIGHTS];

    GLint   id_uniform_specular_intensity = 0;
    GLint   id_uniform_shininess = 0;

    virtual void load_uniforms() override
    {
        shader_perspective_t::load_uniforms();
        id_uniform_observer_pos = uniform_location("observer_pos");
        id_uniform_directional_light.colour = uniform_location("directional_light.colour");
        id_uniform_directional_light.ambient_intensity = uniform_location("directional_light.ambient_intensity");
        id_uniform_directional_light.diffuse_intensity = uniform_location("directional_light.diffuse_intensity");
        id_uniform_directional_light.direction = uniform_location("directional_light.direction");
        id_uniform_point_light_count = uniform_location("point_light_count");
        for(size_t i = 0; i < MAX_POINT_LIGHTS; ++i)
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
        id_uniform_spot_light_count = uniform_location("spot_light_count");
        for(size_t i = 0; i < MAX_SPOT_LIGHTS; ++i)
        {
            char loc_buffer[128] = {0};
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].plight.colour", i);
            id_uniform_spot_light[i].colour = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].plight.ambient_intensity", i);
            id_uniform_spot_light[i].ambient_intensity = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].plight.diffuse_intensity", i);
            id_uniform_spot_light[i].diffuse_intensity = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].plight.position", i);
            id_uniform_spot_light[i].position = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].plight.att_constant", i);
            id_uniform_spot_light[i].att_constant = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].plight.att_linear", i);
            id_uniform_spot_light[i].att_linear = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].plight.att_quadratic", i);
            id_uniform_spot_light[i].att_quadratic = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].direction", i);
            id_uniform_spot_light[i].direction = uniform_location(loc_buffer);
            stbsp_snprintf(loc_buffer, sizeof(loc_buffer), "spot_light[%d].cutoff", i);
            id_uniform_spot_light[i].cos_cutoff = uniform_location(loc_buffer);
        }
        id_uniform_specular_intensity = uniform_location("material.specular_intensity");
        id_uniform_shininess = uniform_location("material.shininess");
    }
};

struct material_t
{
    real32 specular_intensity = 0.f;
    real32 shininess = 1.f;
};

struct light_t
{
    vec3    colour = { 1.f, 1.f, 1.f };
    real32  ambient_intensity = 0.2f;
    real32  diffuse_intensity = 1.0f;  
};

struct light_directional_t : light_t
{
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };
};

struct light_point_t : light_t
{
    vec3        position = { 0.f, 0.f, 0.f };
    // Attenuation coefficients
    GLfloat     att_constant = 0.3f;
    GLfloat     att_linear = 0.2f;
    GLfloat     att_quadratic = 0.1f;
};

struct light_spot_t : light_point_t
{
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };

    void set_cutoff_in_degrees(float degrees) { cos_cutoff = cosf(degrees * KC_DEG2RAD); }
    void set_cutoff_in_radians(float radians) { cos_cutoff = radians; }
    real32 cosine_cutoff() { return cos_cutoff; }

private:
    real32 cos_cutoff = 0.866f;
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
struct mesh_t
{
    // Holds the ID for the VAO, VBO, IBO in the GPU memory
    uint32  id_vao          = 0;
    uint32  id_vbo          = 0;
    uint32  id_ibo          = 0;
    int32   index_count     = 0;
};

/** Handle for texture_t stored in GPU memory */
struct texture_t
{
    GLuint      texture_id  = 0;        // ID for the texture in GPU memory
    int32       width       = 0;        // Width of the texture
    int32       height      = 0;        // Height of the texture
    GLenum      format      = GL_NONE;  // format / bitdepth of texture (GL_RGB would be 3 byte bit depth)
};

struct meshgroup_t
{
    std::vector<mesh_t> meshes;
    std::vector<texture_t> textures;
    std::vector<uint16> mesh_to_texture;
};

/** camera_t properties */
struct camera_t
{
    vec3   position             = { 0.f };            // camera x y z pos in world space 
    vec3   rotation             = { 0.f };            // pitch, yaw, roll - in that order
    vec3   world_up             = { 0.f, 1.f, 0.f };

    vec3   calculated_direction = { 0.f };            // Intuitive direction - direction forward
    vec3   calculated_up        = { 0.f }; 
    vec3   calculated_right     = { 0.f }; 

    real32 movespeed            = 2.f;
    real32 turnspeed            = 0.17f;

    mat4   matrix_perspective   = { 0.f }; // Perspective projection matrix
    mat4   matrix_view          = { 0.f }; // Last calculated view matrix
};

struct gameobject_t
{
    // TODO this is not very data oriented of you
    vec3        pos = {0.f};
    quaternion  orient = identity_quaternion();
    vec3        scale = {1.f,1.f,1.f};
    meshgroup_t   model;
    // collider_t col;
    // int32 flags
    // Tags tags[4]; // primary, secondary, tertiary, quaternary tags
};

struct temp_map_t
{
    // temporary
    gameobject_t mainobject;
    const char* temp_obj_path;
    vec3 cam_start_pos = {0.f};
    vec3 cam_start_rot = {0.f};

    // prob going to stay
    std::vector<light_point_t> pointlights;
    std::vector<light_spot_t> spotlights;
    light_directional_t directionallight;
};

/**  */
/* 
struct Entity
{
    glm::vec3   pos     = glm::vec3(0.f);
    quaternion  rot     = ;
    glm::vec3   scale   = glm::vec3(0.f);
    mesh_t        mesh;
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