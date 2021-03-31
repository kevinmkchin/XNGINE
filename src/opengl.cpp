/**

    Graphics API specific implementations for OpenGL 3.3

*/


/**

    Shader functions

*/

INTERNAL void gl_compile_shader(uint32 program_id, const char* shader_code, GLenum shader_type)
{
    // Create shader on GPU and compile shader
    GLuint id_shader = glCreateShader(shader_type);             // Create an empty shader of given type and get id
    GLint code_length = (GLint) strlen(shader_code);            // from string.h
    glShaderSource(id_shader, 1, &shader_code, &code_length);   // Fill the empty shader with the shader code
    glCompileShader(id_shader);                                 // Compile the shader source
    // Error check
    GLint result = 0;
    GLchar eLog[1024] = {};
    glGetShaderiv(id_shader, GL_COMPILE_STATUS, &result);       // Make sure the shader compiled correctly
    if (!result)
    {
        glGetProgramInfoLog(id_shader, sizeof(eLog), nullptr, eLog);
        con_printf("Error compiling the %d shader: '%s' \n", shader_type, eLog);
        return;
    }
    // Attach to program
    glAttachShader(program_id, id_shader);
}

INTERNAL void gl_create_shader_program(BaseShader& shader, const char* vertex_shader_str, const char* fragment_shader_str)
{
    // Create an empty shader program and get the id
    shader.id_shader_program = glCreateProgram();
    if (!shader.id_shader_program)
    {
        con_printf("Failed to create shader program! Aborting.\n");
        return;
    }
    // Compile and attach the shaders
    gl_compile_shader(shader.id_shader_program, vertex_shader_str, GL_VERTEX_SHADER);
    gl_compile_shader(shader.id_shader_program, fragment_shader_str, GL_FRAGMENT_SHADER);
    // Actually create the exectuable shader program on the graphics card
    glLinkProgram(shader.id_shader_program);
    // Error checking in shader code
    GLint result = 0;
    GLchar eLog[1024] = {};
    glGetProgramiv(shader.id_shader_program, GL_LINK_STATUS, &result); // Make sure the program was created
    if (!result)
    {
        glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
        con_printf("Error linking program: '%s'! Aborting.\n", eLog);
        return;
    }
    // Validate the program will work
    glValidateProgram(shader.id_shader_program);
    glGetProgramiv(shader.id_shader_program, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader.id_shader_program, sizeof(eLog), nullptr, eLog);
        con_printf("Error validating program: '%s'! Aborting.\n", eLog);
        return;
    }

    shader.load_uniforms();
}

INTERNAL void gl_load_shader_program_from_file(BaseShader& shader, const char* vertex_path, const char* fragment_path)
{
    std::string v = FILE_read_file_string(vertex_path);
    std::string f = FILE_read_file_string(fragment_path);
    gl_create_shader_program(shader, v.c_str(), f.c_str());
}

/** Telling opengl to start using this shader program */
INTERNAL void gl_use_shader(BaseShader& shader)
{
    if (shader.id_shader_program == 0)
    {
        con_printf("WARNING: Passed an unloaded shader program to gl_use_shader! Aborting.\n");
        return;
    }
    glUseProgram(shader.id_shader_program);
}

/** Delete the shader program off GPU memory */
INTERNAL void gl_delete_shader(BaseShader& shader)
{
    if (shader.id_shader_program == 0)
    {
        con_printf("WARNING: Passed an unloaded shader program to gl_delete_shader! Aboring.\n");
        return;
    }
    glDeleteProgram(shader.id_shader_program);
}

INTERNAL void gl_bind_model_matrix(BaseShader& shader, const GLfloat* matrix)
{
    glUniformMatrix4fv(shader.id_uniform_model, 1, GL_FALSE, matrix);
}

INTERNAL void gl_bind_view_matrix(PerspectiveShader& shader, const GLfloat* matrix)
{
    glUniformMatrix4fv(shader.id_uniform_view, 1, GL_FALSE, matrix);
}

INTERNAL void gl_bind_projection_matrix(PerspectiveShader& shader, const GLfloat* matrix)
{
    glUniformMatrix4fv(shader.id_uniform_proj_perspective, 1, GL_FALSE, matrix);
}

INTERNAL void gl_bind_projection_matrix(OrthographicShader& shader, const GLfloat* matrix)
{
    glUniformMatrix4fv(shader.id_uniform_proj_orthographic, 1, GL_FALSE, matrix);
}

INTERNAL void gl_bind_directional_light(LightingShader& shader, DirectionalLight& light)
{
    glUniform1f(shader.id_uniform_ambient_intensity,    light.ambient_intensity);
    glUniform3f(shader.id_uniform_ambient_colour,       light.colour.x, light.colour.y, light.colour.z);
    glUniform1f(shader.id_uniform_diffuse_intensity,    light.diffuse_intensity);
    glUniform3f(shader.id_uniform_light_direction,      light.direction.x, light.direction.y, light.direction.z);
}

INTERNAL void gl_bind_material(LightingShader& shader, Material& material)
{
    glUniform1f(shader.id_uniform_specular_intensity, material.specular_intensity);
    glUniform1f(shader.id_uniform_shininess, material.shininess);
}


/**

    VAO and VBO functions

*/

/** Create a Mesh with the given vertices and indices. 
    vertex_attrib_size: vertex coords size (e.g. 3 if x y z)
    texture_attrib_size: texture coords size (e.g. 2 if u v)
    draw_usage: affects optimization; GL_STATIC_DRAW buffer data 
    only set once, GL_DYNAMIC_DRAW if buffer modified repeatedly */
INTERNAL Mesh gl_create_mesh_array(real32* vertices, 
                                   uint32* indices,
                                   uint32 vertices_array_count,
                                   uint32 indices_array_count,
                                   uint8 vertex_attrib_size = 3,
                                   uint8 texture_attrib_size = 2,
                                   uint8 normal_attrib_size = 3,
                                   GLenum draw_usage = GL_STATIC_DRAW)
{
    uint8 stride = vertex_attrib_size + texture_attrib_size + normal_attrib_size;

    Mesh mesh;
    // Need to store to index_count because we need the count of indices when we are drawing in Mesh::render_mesh
    mesh.index_count = indices_array_count;

    glGenVertexArrays(1, &mesh.id_vao); // Defining some space in the GPU for a vertex array and giving you the vao ID
    glBindVertexArray(mesh.id_vao); // Binding a VAO means we are currently operating on that VAO
        // Indentation is to indicate that we are now working within the bound VAO
        glGenBuffers(1, &mesh.id_vbo); // Creating a buffer object inside the bound VAO and returning the ID
        glBindBuffer(GL_ARRAY_BUFFER, mesh.id_vbo); // Bind VBO to operate on that VBO
            /* Connect the vertices data to the actual gl array buffer for this VBO. We need to pass in the size of the data we are passing as well.
            GL_STATIC_DRAW (as opposed to GL_DYNAMIC_DRAW) means we won't be changing these data values in the array. 
            The vertices array does not need to exist anymore after this call because that data will now be stored in the VAO on the GPU. */
            glBufferData(GL_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * vertices_array_count, vertices, draw_usage);
            /* Index is location in VAO of the attribute we are creating this pointer for.
            Size is number of values we are passing in (e.g. size is 3 if x y z).
            Normalized is normalizing the values.
            Stride is the number of values to skip after getting the values we need.
                for example, you could have vertices and colors in the same array
                [ Ax, Ay, Az,  Ar, Ag, Ab,  Bx, By, Bz,  Br, Bg, Bb ]
                    use          stride        use          stride
                In this case, the stride would be 3 because we need to skip 3 values (the color values) to reach the next vertex data.
            Apparently the last parameter is the offset? */
            glVertexAttribPointer(0, vertex_attrib_size, GL_FLOAT, GL_FALSE, sizeof(real32) * stride, 0); // vertex pointer
            glEnableVertexAttribArray(0); // Enabling location in VAO for the attribute
            glVertexAttribPointer(1, texture_attrib_size, GL_FLOAT, GL_FALSE, sizeof(real32) * stride, (void*)(sizeof(real32) * vertex_attrib_size)); // uv coord pointer
            glEnableVertexAttribArray(1);
            if(normal_attrib_size > 0)
            {
                glVertexAttribPointer(2, normal_attrib_size, GL_FLOAT, GL_FALSE, sizeof(real32) * stride, (void*)(sizeof(real32) * (vertex_attrib_size + texture_attrib_size))); // normal pointer
                glEnableVertexAttribArray(2);
            }
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO

        // Index Buffer Object
        glGenBuffers(1, &mesh.id_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.id_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * indices_array_count, indices, draw_usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind the VAO;

    return mesh;
}

/** Overwrite existing buffer data */
INTERNAL void gl_rebind_buffers(Mesh& mesh,
                                real32* vertices, 
                                uint32* indices,
                                uint32 vertices_array_count,
                                uint32 indices_array_count,
                                GLenum draw_usage = GL_DYNAMIC_DRAW) // default to dynamic draw
{
    if(mesh.id_vbo == 0 || mesh.id_ibo == 0)
    {
        return;
    }

    mesh.index_count = indices_array_count;
    glBindVertexArray(mesh.id_vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.id_vbo);
            glBufferData(GL_ARRAY_BUFFER, 4 * vertices_array_count, vertices, draw_usage);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.id_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indices_array_count, indices, draw_usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/** Binds VAO and draws elements. Bind a shader program and texture before calling gl_render_mesh */
INTERNAL void gl_render_mesh(Mesh& mesh)
{
    if (mesh.index_count == 0) // Early out if index_count == 0, nothing to draw
    {
        con_printf("WARNING: Attempting to render a mesh with 0 index count!\n");
        return;
    }

    // Bind VAO, bind VBO, draw elements(indexed draw)
    glBindVertexArray(mesh.id_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.id_ibo);
            glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, nullptr); // Last param could be pointer to indices but no need cuz IBO is already bound
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/** Clearing GPU memory: glDeleteBuffers and glDeleteVertexArrays deletes the buffer
    object and vertex array object off the GPU memory. */
INTERNAL void gl_delete_mesh(Mesh& mesh)
{
    if (mesh.id_ibo != 0)
    {
        glDeleteBuffers(1, &mesh.id_ibo);
        mesh.id_ibo = 0;
    }
    if (mesh.id_vbo != 0)
    {
        glDeleteBuffers(1, &mesh.id_vbo);
        mesh.id_vbo = 0;
    }
    if (mesh.id_vao != 0)
    {
        glDeleteVertexArrays(1, &mesh.id_vao);
        mesh.id_vao = 0;
    }

    mesh.index_count = 0;
}

/**

    Texture 

*/

/** Deletes texture object from GPU memory; resets texture_id, width, height, bit_depth to 0. */
INTERNAL void gl_delete_texture(Texture& texture)
{
    if(texture.texture_id == 0)
    {
        con_printf("WARNING: Attempting to clear a texture with id: 0. This means this texture hasn't been loaded!\n");
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
        con_printf("WARNING: Trying to load a Texture when there is already a texture loaded! Clearing texture first...\n");
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
        gl_load_texture_from_bitmap(texture, (unsigned char*)texture_handle.memory, texture_handle.width, 
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