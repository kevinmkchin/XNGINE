#pragma once

#include <gl/glew.h>

#include "mkc.h"

class Texture
{
public:
	Texture();
	Texture(const char* file_path);
	~Texture();

	/** Loads texture at file_path; generates a new texture object in GPU mem; stores the id
		of the new texture object into texture_id; sets texture parameters; copies texture data
		into the texture object in GPU mem; and generates mip maps automatically. */
	void load_texture();
	void use_texture();
	/** Deletes texture object from GPU mem; resets texture_id, width, height, bit_depth to 0. */
	void clear_texture();

	void set_filepath(const char* in_file_path);

private:
	GLuint texture_id;
	int32 width, height, bit_depth;
	const char* file_path;

};