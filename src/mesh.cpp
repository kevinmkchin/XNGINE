/**  */
INTERNAL Mesh gl_create_mesh_array(real32* vertices, 
                                   uint32* indices,
                                   uint32 vertices_array_count,
                                   uint32 indices_array_count,
                                   uint8 vertex_attrib_size = 3,
                                   uint8 texture_attrib_size = 2,
                                   GLenum draw_usage = GL_STATIC_DRAW)
{
    uint8 stride = vertex_attrib_size + texture_attrib_size;

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
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO

        // Index Buffer Object
        glGenBuffers(1, &mesh.id_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.id_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * indices_array_count, indices, draw_usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind the VAO;

    return mesh;
}

INTERNAL void gl_rebind_buffers(Mesh& mesh,
                                real32* vertices, 
                                uint32* indices,
                                uint32 vertices_array_count,
                                uint32 indices_array_count,
                                GLenum draw_usage = GL_STATIC_DRAW)
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

/**  */
INTERNAL void gl_render_mesh(Mesh& mesh)
{
    if (mesh.index_count == 0) // Early out if index_count == 0, nothing to draw
    {
        printf("WARNING: Attempting to render a mesh with 0 index count!\n");
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
    object and vertex array object off the GPU memory. 
*/
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