#pragma once

/** Represents a Shader Program */
class Shader
{
public:
	Shader();
	~Shader();

	// Pass in shader as a string
	void create_from_strings(const char* vertex_shader_str, const char* fragment_shader_str);
	void create_from_files(const char* vertex_path, const char* fragment_path);

	std::string read_shader_file(const char* path);

	// Gets the location id of uniform variable with the given name
	uint32 get_matrix_model_location_id();
	uint32 get_matrix_view_location_id();
	uint32 get_matrix_projection_location_id();

	/** Calls glUseProgram with this shader program. You can switch out shader programs so you can draw 
		different objects or scenes with different shader programs. */
	void use_shader();

	/** Deletes this shader program from GPU memory. */
	void clear_shader();

private:
	void compile_shader(const char* vertex_shader_str, const char* fragment_shader_str);
	void add_shader(uint32 program_id, const char* shader_code, GLenum shader_type);

private:
	// id for shader program, the model uniform variable, and the projection uniform variable
	uint32 id_shader_program, id_uniform_model, id_uniform_view, id_uniform_projection;

};

