INTERNAL void add_shader(uint32 program_id, const char* shader_code, GLenum shader_type)
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


INTERNAL void compile_shader(ShaderProgram& shader, const char* vertex_shader_str, const char* fragment_shader_str)
{
    shader.id_shader_program = glCreateProgram(); // Creates the shader program and returns the id
    if (!shader.id_shader_program)
    {
        printf("Failed to create shader program.\n");
        return;
    }

    // Compile and attach the shaders
    add_shader(shader.id_shader_program, vertex_shader_str, GL_VERTEX_SHADER);
    add_shader(shader.id_shader_program, fragment_shader_str, GL_FRAGMENT_SHADER);

    // Error checking in shader code
    GLint result = 0;
    GLchar eLog[1024] = {};
    glLinkProgram(shader.id_shader_program); // Actually create the exectuable shader program on the graphics card
    glGetProgramiv(shader.id_shader_program, GL_LINK_STATUS, &result); // Make sure the program was created
    if (!result)
    {
        glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
        printf("Error linking program: '%s' \n", eLog);
        return;
    }
    // Validate the program will work
    glValidateProgram(shader.id_shader_program);
    glGetProgramiv(shader.id_shader_program, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
        printf("Error validating program: '%s' \n", eLog);
        return;
    }

    // UNIFORM VARIABLES
    shader.id_uniform_model = glGetUniformLocation(shader.id_shader_program, "matrix_model");
    shader.id_uniform_view = glGetUniformLocation(shader.id_shader_program, "matrix_view");
    shader.id_uniform_projection = glGetUniformLocation(shader.id_shader_program, "matrix_projection");
}

INTERNAL void init_shader_program_from_strings(ShaderProgram& shader, const char* vertex_shader_str, const char* fragment_shader_str)
{
    compile_shader(shader, vertex_shader_str, fragment_shader_str);
}

INTERNAL void init_shader_program(ShaderProgram& shader, const char* vertex_path, const char* fragment_path)
{
    std::string v = file_read_file_string(vertex_path);
    std::string f = file_read_file_string(fragment_path);
    init_shader_program_from_strings(shader, v.c_str(), f.c_str());
}

/** Telling opengl to start using this shader program */
INTERNAL void use_shader(ShaderProgram& shader)
{
    if (shader.id_shader_program == 0)
    {
        printf("WARNING: Attempting to Shader::use_shader when there is no shader program loaded!\n");
        return;
    }
    glUseProgram(shader.id_shader_program);
}

/** Delete the shader program off GPU memory */
INTERNAL void gl_clear_shader(ShaderProgram& shader)
{
    if (shader.id_shader_program == 0)
    {
        printf("WARNING: Attempting to Shader::clear_shader when there is no shader program loaded!\n");
        return;
    }
    glDeleteProgram(shader.id_shader_program);
    shader.id_shader_program = 0;
    shader.id_uniform_model = 0;
    shader.id_uniform_projection = 0;
}