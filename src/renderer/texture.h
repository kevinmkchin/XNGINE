#pragma once

#include <vector>
#include <string>
#include "../gamedefine.h"
#include "GL/glew.h"

/** Handle for texture stored in GPU memory */
struct texture_t
{
    GLuint  texture_id  = 0;        // ID for the texture in GPU memory
    i32     width       = 0;        // Width of the texture
    i32     height      = 0;        // Height of the texture
    GLenum  format      = GL_NONE;  // format / bitdepth of texture (GL_RGB would be 3 byte bit depth)

    /** Loads texture from bitmap; generates a new texture object in GPU mem; store the id
    of the new texture object into texture.texture_id; sets texture parameters; copies texture data
    into the texture object in GPU mem; and generates mip maps automatically. */
    static void gl_create_from_bitmap(texture_t&        texture,
                                      unsigned char*    bitmap,
                                      u32               bitmap_width,
                                      u32               bitmap_height,
                                      GLenum            target_format,
                                      GLenum            source_format);

    /** Loads texture at file_path; generates a new texture object in GPU mem; stores the id
    of the new texture object into texture.texture_id; sets texture parameters; copies
    texture data into the texture object in GPU mem; and generates mip maps automatically. */
    static void gl_create_from_file(texture_t&    texture,
                                    const char* texture_file_path);

    /** Deletes texture object from GPU memory; resets texture_id, width, height, bit_depth to 0. */
    static void gl_delete(texture_t& texture);

    // Binds this texture to texture_t Unit 1
    // When we are drawing and try to access texture_t Unit 0, we will be accessing this texture now
    // Anything drawn after this point will use these textures
    /** If we have multiple texture samplers or multiple texture_t Units, ensure sampler2D uniforms
        know which texture_t Unit to access via:
            glUnifrom1i(<texture_sampler_uniform_id>, <texture_unit_number>); */
    void gl_use_texture() const;
};

/** Handle for cubemap stored in GPU memory */
struct cubemap_t : public texture_t
{
    static void gl_create_from_files(cubemap_t& cubemap, const std::vector<std::string>& faces_paths);
};