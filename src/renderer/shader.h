#pragma once

#include "../gamedefine.h"
#include <unordered_map>
#include <GL/glew.h>

/** Handle for Shader Program stored in GPU memory */
struct shader_t
{
    static void gl_use_shader(shader_t& shader);

    static void gl_delete_shader(shader_t& shader);

    static void gl_load_shader_program_from_file(shader_t& shader, const char* vertex_path, const char* fragment_path);

    static void gl_load_shader_program_from_file(shader_t& shader, const char* vertex_path, const char* geometry_path, const char* fragment_path);

    static void gl_create_shader_program(shader_t& shader, const char* vertex_shader_str, const char* fragment_shader_str);

    static void gl_create_shader_program(shader_t& shader, const char* vertex_shader_str, const char* geometry_shader_str, const char* fragment_shader_str);

    static void gl_load_compute_shader_program_from_file(shader_t& shader, const char* compute_path);

    static void gl_create_compute_shader_program(shader_t& shader, const char* compute_shader_str);

    void gl_bind_1i(const char* uniform_name, GLint v0) const;
    void gl_bind_2i(const char* uniform_name, GLint v0, GLint v1) const;
    void gl_bind_3i(const char* uniform_name, GLint v0, GLint v1, GLint v2) const;
    void gl_bind_4i(const char* uniform_name, GLint v0, GLint v1, GLint v2, GLint v3) const;
    void gl_bind_1f(const char* uniform_name, GLfloat v0) const;
    void gl_bind_2f(const char* uniform_name, GLfloat v0, GLfloat v1) const;
    void gl_bind_3f(const char* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2) const;
    void gl_bind_4f(const char* uniform_name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const;
    void gl_bind_matrix3fv(const char* uniform_name, GLsizei count, const GLfloat* value) const;
    void gl_bind_matrix4fv(const char* uniform_name, GLsizei count, const GLfloat* value) const;

    i32 get_cached_uniform_location(const char* uniform_name) const;
private:
    GLuint id_shader_program = 0; // id of this shader program in GPU memory

    std::unordered_map<std::string, i32> uniform_locations;

    static void cache_uniform_locations(shader_t& shader);

    static void cache_uniform_location(shader_t& shader, const char* uniform_name);

    void warning_uniform_not_found(const char* uniform_name) const;

    // Create shader on GPU and compile shader
    static void gl_compile_shader(u32 program_id, const char* shader_code, GLenum shader_type);

    // Return true if there was a compile error
    static bool gl_check_error_and_validate(GLuint program_id);
};
