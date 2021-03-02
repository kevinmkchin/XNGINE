Shader::Shader()
	: id_shader_program(0)
	, id_uniform_model(0)
	, id_uniform_view(0)
	, id_uniform_projection(0)
{

}

Shader::~Shader()
{
	clear_shader();
}

void Shader::create_from_strings(const char* vertex_shader_str, const char* fragment_shader_str)
{
	compile_shader(vertex_shader_str, fragment_shader_str);
}

void Shader::create_from_files(const char* vertex_path, const char* fragment_path)
{
	std::string v = read_shader_file(vertex_path);
	std::string f = read_shader_file(fragment_path);
	create_from_strings(v.c_str(), f.c_str());
}

std::string Shader::read_shader_file(const char* path)
{
	std::string shader_content;

	std::ifstream file_stream(path, std::ios::in);
	if (file_stream.is_open() == false)
	{
		printf("Failed to read %s! Shader file doesn't exist.\n", path);
	}

	std::string line = "";
	while (file_stream.eof() == false)
	{
		std::getline(file_stream, line);
		shader_content.append(line + "\n");
	}

	file_stream.close();

	return shader_content;
}

uint32 Shader::get_matrix_model_location_id()
{
	return id_uniform_model;
}

uint32 Shader::get_matrix_view_location_id()
{
	return id_uniform_view;
}

uint32 Shader::get_matrix_projection_location_id()
{
	return id_uniform_projection;
}

void Shader::use_shader()
{
	if (id_shader_program == 0)
	{
		printf("WARNING: Attempting to Shader::use_shader when there is no shader program loaded!\n");
		return;
	}

	glUseProgram(id_shader_program); // Telling opengl to start using this shader program
}

void Shader::clear_shader()
{
	if (id_shader_program == 0)
	{
		printf("WARNING: Attempting to Shader::clear_shader when there is no shader program loaded!\n");
		return;
	}
	// Delete the shader program off GPU memory
	glDeleteProgram(id_shader_program);
	id_shader_program = 0;
	// Reset uniform variable location ids
	id_uniform_model = 0;
	id_uniform_projection = 0;
}

void Shader::compile_shader(const char* vertex_shader_str, const char* fragment_shader_str)
{
	id_shader_program = glCreateProgram(); // Creates the shader program and returns the id
	if (!id_shader_program)
	{
		printf("Failed to create shader program.\n");
		return;
	}

	// Compile and attach the shaders
	add_shader(id_shader_program, vertex_shader_str, GL_VERTEX_SHADER);
	add_shader(id_shader_program, fragment_shader_str, GL_FRAGMENT_SHADER);

	// Error checking in shader code
	// Pretty important because intellisense isn't going to check shader code
	GLint result = 0;
	GLchar eLog[1024] = { };
	glLinkProgram(id_shader_program); // Actually create the exectuable shader program on the graphics card
	glGetProgramiv(id_shader_program, GL_LINK_STATUS, &result); // Make sure the program was created
	if (!result)
	{
		glGetProgramInfoLog(id_shader_program, sizeof(eLog), nullptr, eLog);
		printf("Error linking program: '%s' \n", eLog);
		return;
	}
	// Validate the program will work
	glValidateProgram(id_shader_program);
	glGetProgramiv(id_shader_program, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(id_shader_program, sizeof(eLog), nullptr, eLog);
		printf("Error validating program: '%s' \n", eLog);
		return;
	}

	// UNIFORM VARIABLES
	id_uniform_model = glGetUniformLocation(id_shader_program, "matrix_model");
	id_uniform_view = glGetUniformLocation(id_shader_program, "matrix_view");
	id_uniform_projection = glGetUniformLocation(id_shader_program, "matrix_projection");
}

void Shader::add_shader(uint32 program_id, const char* shader_code, GLenum shader_type)
{
	GLuint id_shader = glCreateShader(shader_type); // Create an empty shader of given type and get id

	const GLchar* the_code[1];
	the_code[0] = shader_code;

	GLint code_length[1];
	code_length[0] = (GLint) strlen(shader_code); // from string.h

	glShaderSource(id_shader, 1, the_code, code_length); // Fill the empty shader with the shader code
	glCompileShader(id_shader); // Compile the shader source

	// Error check
	GLint result = 0;
	GLchar eLog[1024] = { };
	glGetShaderiv(id_shader, GL_COMPILE_STATUS, &result); // Make sure the shader compiled correctly
	if (!result)
	{
		glGetProgramInfoLog(id_shader, sizeof(eLog), nullptr, eLog);
		printf("Error compiling the %d shader: '%s' \n", shader_type, eLog);
		return;
	}

	// Attach to program
	glAttachShader(program_id, id_shader);
}
