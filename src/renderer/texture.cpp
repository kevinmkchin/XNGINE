#include <unordered_map>
#include "texture.h"
#include "../runtime/memory_handle.h"
#include "../core/file_system.h"
#include "../debugging/console.h"

internal std::unordered_map<std::string, texture_t> gpu_loaded_textures;

void texture_t::gl_create_from_bitmap(texture_t&        texture,
                                      unsigned char*    bitmap,
                                      u32               bitmap_width,
                                      u32               bitmap_height,
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
                                    const char*   texture_file_path)
{
    auto texture_already_loaded = gpu_loaded_textures.find(std::string(texture_file_path));
    if(texture_already_loaded != gpu_loaded_textures.end())
    {
        texture = texture_already_loaded->second;
        return;
    }

    bitmap_handle_t texture_handle;
    read_image(texture_handle, texture_file_path);
    gl_create_from_bitmap(texture, (unsigned char*)texture_handle.memory, texture_handle.width,
                          texture_handle.height, GL_RGBA, (texture_handle.bit_depth == 3 ? GL_RGB : GL_RGBA));
    free_image(texture_handle); // texture data has been copied to GPU memory, so we can free image from memory

    gpu_loaded_textures[std::string(texture_file_path)] = texture;
}

void texture_t::gl_delete(texture_t& texture)
{
    if(texture.texture_id == 0)
    {
        console_printf("WARNING: Attempting to clear a texture with id: 0. This means this texture hasn't been loaded!\n");
        return;
    }
    glDeleteTextures(1, &texture.texture_id);

    // TODO find texture from loaded textures and delete

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



void cubemap_t::gl_create_from_files(cubemap_t& cubemap, const std::vector<std::string>& faces_paths)
{
    glGenTextures(1, &cubemap.texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.texture_id);

    for(size_t i = 0; i < 6; ++i)
    {
        bitmap_handle_t face_handle;
        read_image(face_handle, faces_paths[i].c_str());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, face_handle.width, face_handle.height,
                     0,(face_handle.bit_depth == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, face_handle.memory);
        free_image(face_handle);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
