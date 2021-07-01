#include "texture.h"
#include "../runtime/memory_handle.h"
#include "../core/file_system.h"
#include "../debugging/console.h"

void texture_t::gl_create_from_bitmap(texture_t&        texture,
                                      unsigned char*    bitmap,
                                      u32            bitmap_width,
                                      u32            bitmap_height,
                                      GLenum            target_format,
                                      GLenum            source_format)
{
    if(texture.texture_id != 0)
    {
        console_printf("WARNING: Trying to load a texture_t when there is already a texture loaded! Clearing texture first...\n");
        texture_t::gl_delete(texture);
    }

    texture.width = bitmap_width;
    texture.height = bitmap_height;
    texture.format = source_format;

    glGenTextures(1, &texture.texture_id);                                  // generate texture and grab texture id
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // filtering (e.g. GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
            GL_TEXTURE_2D,                                                  // texture target type
            0,                                                              // level-of-detail number n = n-th mipmap reduction image
            target_format,                                                  // format of data to store (target): num of color components
            bitmap_width,                                                   // texture width
            bitmap_height,                                                  // texture height
            0,                                                              // must be 0 (legacy)
            source_format,                                                  // format of data being loaded (source)
            GL_UNSIGNED_BYTE,                                               // data type of the texture data
            bitmap);                                                        // data
    glGenerateMipmap(GL_TEXTURE_2D);                                    // generate mip maps automatically
    glBindTexture(GL_TEXTURE_2D, 0);
}

void texture_t::gl_create_from_file(texture_t&    texture,
                                          const char* texture_file_path)
{
    bitmap_handle_t texture_handle;
    read_image(texture_handle, texture_file_path);
    gl_create_from_bitmap(texture, (unsigned char*)texture_handle.memory, texture_handle.width,
                                texture_handle.height, GL_RGBA, (texture_handle.bit_depth == 3 ? GL_RGB : GL_RGBA));
    free_image(texture_handle); // texture data has been copied to GPU memory, so we can free image from memory
}

void texture_t::gl_delete(texture_t& texture)
{
    if(texture.texture_id == 0)
    {
        console_printf("WARNING: Attempting to clear a texture with id: 0. This means this texture hasn't been loaded!\n");
        return;
    }
    glDeleteTextures(1, &texture.texture_id);
    texture.texture_id = 0;
    texture.width = 0;
    texture.height = 0;
    texture.format = GL_NONE;
}

void texture_t::gl_use_texture() const
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}