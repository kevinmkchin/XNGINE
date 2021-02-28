#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
	: texture_id(0)
	, width(0)
	, height(0)
	, bit_depth(0)
	, file_path("")
{

}

Texture::Texture(const char* file_path)
	: texture_id(0)
	, width(0)
	, height(0)
	, bit_depth(0)
	, file_path(file_path)
{

}

void Texture::load_texture()
{
	if(texture_id != 0)
	{
		printf("WARNING: Called Texture::load_texture when there is already a texture loaded! Clearing texture first...\n");
		clear_texture();
	}

	unsigned char* texture_data = stbi_load(file_path, &width, &height, &bit_depth, 0);
	if(texture_data == nullptr)
	{
		printf("Failed to find texture file at: %s\n", file_path);
		return;
	}

	glGenTextures(1, &texture_id); // generate texture and grab texture id
	glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(
			GL_TEXTURE_2D, 		// texture target type
			0,					// level-of-detail number n = n-th mipmap reduction image
			GL_RGBA,			// format of data to store: num of color components
			width,				// texture width
			height,				// texture height
			0,					// must be 0 (legacy)
			GL_RGBA,			// format of data being loaded
			GL_UNSIGNED_BYTE,	// data type of the texture data
			texture_data);		// data
		glGenerateMipmap(GL_TEXTURE_2D); // generate mip maps automatically
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(texture_data); // texture data has been copied to GPU memory, so we can free image from memory
}

void Texture::use_texture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id); // Binds this texture to the above Texture Unit
	// When we are drawing and try to access Texture Unit 0, we will be accessing this texture now
	// Anything drawn after this point will use these textures

	/** If we have multiple texture samplers or multiple Texture Units, ensure sampler2D uniforms
		know which Texture Unit to access via:
			glUnifrom1i(<texture_sampler_uniform_id>, <texture_unit_number>);
	*/
}

void Texture::clear_texture()
{
	if(texture_id == 0)
	{
		printf("WARNING: Attempting to Texture::clear_texture when there is no texture loaded!\n");
		return;
	}
	glDeleteTextures(1, &texture_id);
	texture_id = 0;
	width = 0;
	height = 0;
	bit_depth = 0;
}

void Texture::set_filepath(const char* in_file_path)
{
	file_path = in_file_path;
}

Texture::~Texture()
{
	if(texture_id != 0)
	{
		clear_texture();
	}
}