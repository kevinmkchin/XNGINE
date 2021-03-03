#pragma once

// Core game engine objects

struct Texture
{
	GLuint 		texture_id 	= 0;	// ID for the texture in GPU memory
	int32 		width 		= 0;	// Width of the texture
	int32 		height 		= 0;	// Height of the texture
	int32		bit_depth 	= 0;
	const char* file_path 	= "";	// File path on disk to load the texture image from
};

struct Mesh
{
	// Holds the ID for the VAO, VBO, IBO in the GPU memory
	uint32 	id_vao;
	uint32 	id_vbo;
	uint32 	id_ibo;
	int32	index_count;
};

struct Camera
{
	glm::vec3 position = glm::vec3(0.f);			// camera x y z pos in world space 
	glm::vec3 rotation = glm::vec3(0.f);			// pitch, yaw, roll - in that order
	glm::vec3 world_up = glm::vec3(0.f, 1.f, 0.f);

	glm::vec3 calculated_direction = glm::vec3(0.f);// Intuitive direction - direction forward
	glm::vec3 calculated_up = glm::vec3(0.f);
	glm::vec3 calculated_right = glm::vec3(0.f);

	real32 movespeed = 2.f;
	real32 turnspeed = 0.17f;
};

struct ShaderProgram
{
	uint32 id_shader_program 		= 0;	// id of this shader program in GPU memory
	uint32 id_uniform_model			= 0;	// location id for the model matrix uniform variable
	uint32 id_uniform_view			= 0;	// location id for the view matrix uniform variable
	uint32 id_uniform_projection	= 0;	// location id for hte projection matrix uniform variable
};