#pragma once

#include "../game_defines.h"
#include "GL/glew.h"

/** Stores mesh { VAO, VBO, IBO } info. Handle for VAO on GPU memory
 *  Holds the ID for the VAO, VBO, IBO in the GPU memory
*/
struct mesh_t
{
    u32  id_vao          = 0;
    u32  id_vbo          = 0;
    u32  id_ibo          = 0;
    u32  indices_count   = 0;

    /** Create a mesh_t with the given vertices and indices.
    vertex_attrib_size: vertex coords size (e.g. 3 if x y z)
    texture_attrib_size: texture coords size (e.g. 2 if u v)
    draw_usage: affects optimization; GL_STATIC_DRAW buffer data
    only set once, GL_DYNAMIC_DRAW if buffer modified repeatedly */
    static void gl_create_mesh(mesh_t& mesh,
                               float* vertices,
                               u32* indices,
                               u32 vertices_array_count,
                               u32 indices_array_count,
                               u8 vertex_attrib_size = 3,
                               u8 texture_attrib_size = 2,
                               u8 normal_attrib_size = 3,
                               GLenum draw_usage = GL_STATIC_DRAW);

    /** Clearing GPU memory: glDeleteBuffers and glDeleteVertexArrays deletes the buffer
        object and vertex array object off the GPU memory. */
    static void gl_delete_mesh(mesh_t& mesh);

    /** Binds VAO and draws elements. Bind a shader program and texture
        before calling gl_render_mesh */
    void gl_render_mesh(GLenum render_mode = GL_TRIANGLES) const;

    /** Overwrite existing buffer data */
    void gl_rebind_buffer_objects(float* vertices,
                                  u32* indices,
                                  u32 vertices_array_count,
                                  u32 indices_array_count,
                                  GLenum draw_usage = GL_DYNAMIC_DRAW);
};
