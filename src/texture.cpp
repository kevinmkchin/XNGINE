// function prototypes
INTERNAL void gl_load_texture(Texture& texture);
INTERNAL void gl_use_texture(Texture& texture);
INTERNAL void gl_delete_texture(Texture& texture);

/**

*/
INTERNAL void gl_load_bitmap(Texture& texture, unsigned char* bitmap, int32 bitmap_width, int32 bitmap_height, GLenum target_format, GLenum source_format)
{
	glGenTextures(1, &texture.texture_id);									// generate texture and grab texture id
	glBindTexture(GL_TEXTURE_2D, texture.texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(
			GL_TEXTURE_2D,													// texture target type
			0,																// level-of-detail number n = n-th mipmap reduction image
			target_format,													// format of data to store (target): num of color components
			bitmap_width,													// texture width
			bitmap_height,													// texture height
			0,																// must be 0 (legacy)
			source_format,													// format of data being loaded (source)
			GL_UNSIGNED_BYTE,												// data type of the texture data
			bitmap);														// data
		glGenerateMipmap(GL_TEXTURE_2D);									// generate mip maps automatically
	glBindTexture(GL_TEXTURE_2D, 0);
}

/** Loads texture at file_path; generates a new texture object in GPU mem; stores the id
	of the new texture object into texture_id; sets texture parameters; copies texture data
	into the texture object in GPU mem; and generates mip maps automatically. 
*/
// TODO stbi_load should be moved somewhere else. Loading the bitmap should be somewhere else.
INTERNAL void gl_load_texture(Texture& texture)
{
	if(texture.texture_id != 0)
	{
		printf("WARNING: Called Texture::load_texture when there is already a texture loaded! Clearing texture first...\n");
		gl_delete_texture(texture);
	}

	stbi_set_flip_vertically_on_load(true);
	unsigned char* texture_data = stbi_load(texture.file_path, &texture.width, &texture.height, &texture.bit_depth, 0);
	if(texture_data == nullptr)
	{
		printf("Failed to find texture file at: %s\n", texture.file_path);
		return;
	}
	gl_load_bitmap(texture, texture_data, texture.width, texture.height, GL_RGBA, 
		(texture.bit_depth == 3 ? GL_RGB : GL_RGBA));
	stbi_image_free(texture_data); // texture data has been copied to GPU memory, so we can free image from memory
}

// Binds this texture to Texture Unit 0
// When we are drawing and try to access Texture Unit 0, we will be accessing this texture now
// Anything drawn after this point will use these textures
/** If we have multiple texture samplers or multiple Texture Units, ensure sampler2D uniforms
	know which Texture Unit to access via:
		glUnifrom1i(<texture_sampler_uniform_id>, <texture_unit_number>);
*/
INTERNAL void gl_use_texture(Texture& texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.texture_id); 
}

/** Deletes texture object from GPU memory; resets texture_id, width, height, bit_depth to 0. 
*/
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
	texture.bit_depth = 0;
}