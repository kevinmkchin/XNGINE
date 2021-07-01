#include "shader.h"
#include "../modules/console.h"
#include "../runtime/file_system.h"

/** Telling opengl to start using this shader program */
void shader_t::gl_use_shader(shader_t& shader)
{
    if (shader.id_shader_program == 0)
    {
        console_printf("WARNING: Passed an unloaded shader program to gl_use_shader! Aborting.\n");
        return;
    }
    glUseProgram(shader.id_shader_program);
}

/** Delete the shader program off GPU memory */
void shader_t::gl_delete_shader(shader_t& shader)
{
    if (shader.id_shader_program == 0)
    {
        console_printf("WARNING: Passed an unloaded shader program to gl_delete_shader! Aboring.\n");
        return;
    }
    glDeleteProgram(shader.id_shader_program);
}

void shader_t::gl_create_shader_program(shader_t& shader, const char* vertex_shader_str, const char* fragment_shader_str)
{
    // Create an empty shader program and get the id
    shader.id_shader_program = glCreateProgram();
    if (!shader.id_shader_program)
    {
        console_printf("Failed to create shader program! Aborting.\n");
        return;
    }
    // Compile and attach the shaders
    gl_compile_shader(shader.id_shader_program, vertex_shader_str, GL_VERTEX_SHADER);
    gl_compile_shader(shader.id_shader_program, fragment_shader_str, GL_FRAGMENT_SHADER);
    // Actually create the exectuable shader program on the graphics card
    glLinkProgram(shader.id_shader_program);

    GLint result = 0;
    GLchar eLog[1024] = {};
    glGetProgramiv(shader.id_shader_program, GL_LINK_STATUS, &result); // Make sure the program was created
    if (!result)
    {
        glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
        console_printf("Error linking program: '%s'! Aborting.\n", eLog);
        return;
    }

    // Validate the program will work
    // glValidateProgram(shader.id_shader_program);
    // glGetProgramiv(shader.id_shader_program, GL_VALIDATE_STATUS, &result);
    // if (!result)
    // {
    //     glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
    //     console_printf("Error validating program: '%s'! Aborting.\n", eLog);
    //     return;
    // }

    shader_t::cache_uniform_locations(shader);
}

void shader_t::gl_create_shader_program(shader_t& shader, const char* vertex_shader_str, const char* geometry_shader_str, const char* fragment_shader_str)
{
    // Create an empty shader program and get the id
    shader.id_shader_program = glCreateProgram();
    if (!shader.id_shader_program)
    {
        console_printf("Failed to create shader program! Aborting.\n");
        return;
    }
    // Compile and attach the shaders
    gl_compile_shader(shader.id_shader_program, vertex_shader_str, GL_VERTEX_SHADER);
    gl_compile_shader(shader.id_shader_program, geometry_shader_str, GL_GEOMETRY_SHADER);
    gl_compile_shader(shader.id_shader_program, fragment_shader_str, GL_FRAGMENT_SHADER);
    // Actually create the exectuable shader program on the graphics card
    glLinkProgram(shader.id_shader_program);
    // Error checking in shader code
    GLint result = 0;
    GLchar eLog[1024] = {};
    glGetProgramiv(shader.id_shader_program, GL_LINK_STATUS, &result); // Make sure the program was created
    if (!result)
    {
        glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
        console_printf("Error linking program: '%s'! Aborting.\n", eLog);
        return;
    }

    // Validate the program will work
//    glValidateProgram(shader.id_shader_program);
//    glGetProgramiv(shader.id_shader_program, GL_VALIDATE_STATUS, &result);
//    if (!result)
//    {
//        glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
//        console_printf("Error validating program: '%s'! Aborting.\n", eLog);
//        return;
//    }

    shader_t::cache_uniform_locations(shader);
}

void shader_t::gl_load_shader_program_from_file(shader_t& shader, const char* vertex_path, const char* fragment_path)
{
    std::string v = read_file_string(vertex_path);
    std::string f = read_file_string(fragment_path);
    gl_create_shader_program(shader, v.c_str(), f.c_str());
}

void shader_t::gl_load_shader_program_from_file(shader_t& shader, const char* vertex_path, const char* geometry_path, const char* fragment_path)
{
    std::string v = read_file_string(vertex_path);
    std::string g = read_file_string(geometry_path);
    std::string f = read_file_string(fragment_path);
    gl_create_shader_program(shader, v.c_str(), g.c_str(), f.c_str());
}

void shader_t::cache_uniform_locations(shader_t &shader)
{
    shader.uniform_locations.clear();

    GLint longest_uniform_name_length;
    GLint number_of_uniforms;
    glGetProgramiv(shader.id_shader_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &longest_uniform_name_length);
    glGetProgramiv(shader.id_shader_program, GL_ACTIVE_UNIFORMS, &number_of_uniforms);
    console_printf("number of active uniforms for shader %d:  %d\n", shader.id_shader_program, number_of_uniforms);

    GLint readlength;
    GLint size;
    GLenum type;
    GLchar uniform_name[128]; ASSERT(longest_uniform_name_length <= 128)

    /**If one or more elements of an array are active, the name of the array is returned in name, the type is returned
     * in type, and the size parameter returns the highest array element index used, plus one, as determined by the compiler
     * and/or linker. Only one active uniform variable will be reported for a uniform array.
    */
    for (GLint i = 0; i < number_of_uniforms; ++i)
    {
        // TODO NOTE glGetActiveUniform won't work with non-struct arrays https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetActiveUniform.xhtml
        glGetActiveUniform(shader.id_shader_program, i, longest_uniform_name_length, &readlength, &size, &type, uniform_name);
        cache_uniform_location(shader, uniform_name);
    }
}

void shader_t::cache_uniform_location(shader_t& shader, const char *uniform_name)
{
    int32 location = glGetUniformLocation(shader.id_shader_program, uniform_name);
    if (location != 0xffffffff)
    {
        shader.uniform_locations[std::string(uniform_name)] = location;
    }
    else
    {
        console_printf("Warning! Unable to get the location of uniform '%s' for shader id %d...\n", uniform_name, shader.id_shader_program);
    }
}

void shader_t::gl_compile_shader(uint32 program_id, const char* shader_code, GLenum shader_type)
{
    GLuint id_shader = glCreateShader(shader_type);             // Create an empty shader of given type and get id
    GLint code_length = (GLint) strlen(shader_code);
    glShaderSource(id_shader, 1, &shader_code, &code_length);   // Fill the empty shader with the shader code
    glCompileShader(id_shader);                                 // Compile the shader source

    GLint result = 0;
    GLchar eLog[1024] = {};
    glGetShaderiv(id_shader, GL_COMPILE_STATUS, &result);       // Make sure the shader compiled correctly
    if (!result)
    {
        glGetProgramInfoLog(id_shader, sizeof(eLog), nullptr, eLog);
        console_printf("Error compiling the %d shader: '%s' \n", shader_type, eLog);
        return;
    }

    glAttachShader(program_id, id_shader);
}

void shader_t::gl_bind_1i(const char* uniform_name, GLint v0)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform1i(location, v0);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_2i(const char* uniform_name, GLint v0, GLint v1)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform2i(location, v0, v1);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_3i(const char* uniform_name, GLint v0, GLint v1, GLint v2)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform3i(location, v0, v1, v2);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_4i(const char* uniform_name, GLint v0, GLint v1, GLint v2, GLint v3)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform4i(location, v0, v1, v2, v3);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_1f(const char* uniform_name, GLfloat v0)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform1f(location, v0);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_2f(const char* uniform_name, GLfloat v0, GLfloat v1)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform2f(location, v0, v1);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_3f(const char* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform3f(location, v0, v1, v2);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_4f(const char* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniform4f(location, v0, v1, v2, v3);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_matrix3fv(const char* uniform_name, const GLsizei count, const GLfloat* value)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniformMatrix3fv(location, count, GL_FALSE, value);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

void shader_t::gl_bind_matrix4fv(const char* uniform_name, const GLsizei count, const GLfloat* value)
{
    int32 location = get_cached_uniform_location(uniform_name);
    if(location >= 0)
    {
        glUniformMatrix4fv(location, count, GL_FALSE, value);
    }
    else
    {
        warning_uniform_not_found(uniform_name);
    }
}

int32 shader_t::get_cached_uniform_location(const char* uniform_name)
{
    auto location_iter = uniform_locations.find(uniform_name);
    if(location_iter != uniform_locations.end())
    {
        return location_iter->second;
    }
    return -1;
}

void shader_t::warning_uniform_not_found(const char* uniform_name) const
{
    console_printf("Warning: Uniform '%s' doesn't exist or isn't active on shader %d.\n", uniform_name, id_shader_program);
}
