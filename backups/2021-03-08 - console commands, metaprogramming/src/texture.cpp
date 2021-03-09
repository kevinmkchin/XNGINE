/** Deletes texture object from GPU memory; resets texture_id, width, height, bit_depth to 0. */
INTERNAL void gl_delete_texture(Texture& texture)
{
    if(texture.texture_id == 0)
    {
        printf("WARNING: Attempting to clear a texture with id: 0. This means this texture hasn't been loaded!\n");
        return;
    }
    glDeleteTextures(1, &texture.texture_id);
    texture.texture_id = 0;
    texture.width = 0;
    texture.height = 0;
    texture.format = GL_NONE;
}

/** Loads texture from bitmap; generates a new texture object in GPU mem; store the id
    of the new texture object into texture.texture_id; sets texture parameters; copies texture data
    into the texture object in GPU mem; and generates mip maps automatically. */
INTERNAL void gl_load_texture_from_bitmap(Texture&          texture,
                                          unsigned char*    bitmap,
                                          uint32            bitmap_width,
                                          uint32            bitmap_height,
                                          GLenum            target_format,
                                          GLenum            source_format)
{
    if(texture.texture_id != 0)
    {
        printf("WARNING: Trying to load a Texture when there is already a texture loaded! Clearing texture first...\n");
        gl_delete_texture(texture);
    }

    texture.width = bitmap_width;
    texture.height = bitmap_height;
    texture.format = source_format;

    glGenTextures(1, &texture.texture_id);                                  // generate texture and grab texture id
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // filtering
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

/** Loads texture at file_path; generates a new texture object in GPU mem; stores the id
    of the new texture object into texture.texture_id; sets texture parameters; copies 
    texture data into the texture object in GPU mem; and generates mip maps automatically. */
INTERNAL void gl_load_texture_from_file(Texture&    texture,
                                        const char* texture_file_path)
{
    BitmapHandle texture_handle;
    FILE_read_image(texture_handle, texture_file_path);
        gl_load_texture_from_bitmap(texture, texture_handle.memory, texture_handle.width, 
            texture_handle.height, GL_RGBA, (texture_handle.bit_depth == 3 ? GL_RGB : GL_RGBA));
    FILE_free_image(texture_handle); // texture data has been copied to GPU memory, so we can free image from memory
}

// Binds this texture to Texture Unit 0
// When we are drawing and try to access Texture Unit 0, we will be accessing this texture now
// Anything drawn after this point will use these textures
/** If we have multiple texture samplers or multiple Texture Units, ensure sampler2D uniforms
    know which Texture Unit to access via:
        glUnifrom1i(<texture_sampler_uniform_id>, <texture_unit_number>); */
INTERNAL void gl_use_texture(Texture& texture)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.texture_id); 
}