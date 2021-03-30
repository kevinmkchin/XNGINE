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
struct ShaderProgram
{
    GLuint  id_shader_program = 0;    // id of this shader program in GPU memory\

    GLint   id_uniform_model = 0;    // location id for the model matrix uniform variable
    GLint   id_uniform_view = 0;    // location id for the view matrix uniform variable
    GLint   id_uniform_projection = 0;    // location id for hte projection matrix uniform variable

    GLint   id_uniform_observer_pos = 0;

    GLint   id_uniform_ambient_intensity = 0;
    GLint   id_uniform_ambient_colour = 0;
    GLint   id_uniform_diffuse_intensity = 0;
    GLint   id_uniform_light_direction = 0;
    GLint   id_uniform_specular_intensity = 0;
    GLint   id_uniform_shininess = 0;
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