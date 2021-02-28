#include "Mesh.h"
#include <GL/glew.h>

Mesh::Mesh()
	: id_vao(0)
	, id_vbo(0)
	, id_ibo(0)
	, index_count(0)
{

}

Mesh::~Mesh()
{
	clear_mesh();
}

void Mesh::create_mesh(float* vertices, uint32* indices, uint32 num_of_vertices, uint32 num_of_indices)
{
	// Need to store to index_count because we need the count of indices when we are drawing in Mesh::render_mesh
	index_count = num_of_indices;

	glGenVertexArrays(1, &id_vao); // Defining some space in the GPU for a vertex array and giving you the vao ID
	glBindVertexArray(id_vao); // Binding a VAO means we are currently operating on that VAO
		// Indentation is to indicate that we are now working within the bound VAO
		glGenBuffers(1, &id_vbo); // Creating a buffer object inside the bound VAO and returning the ID
		glBindBuffer(GL_ARRAY_BUFFER, id_vbo); // Bind VBO to operate on that VBO
			/* Connect the vertices data to the actual gl array buffer for this VBO. We need to pass in the size of the data we are passing as well.
			GL_STATIC_DRAW (as opposed to GL_DYNAMIC_DRAW) means we won't be changing these data values in the array. 
			The vertices array does not need to exist anymore after this call because that data will now be stored in the VAO on the GPU. */
			glBufferData(GL_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * num_of_vertices, vertices, GL_STATIC_DRAW); // NOTE: 12 instead of 9 now because 12 elements in vertices
			/* Index is location in VAO of the attribute we are creating this pointer for.
			Size is number of values we are passing in (e.g. size is 3 if x y z).
			Normalized is normalizing the values.
			Stride is the number of values to skip after getting the values we need.
				for example, you could have vertices and colors in the same array
				[ Ax, Ay, Az,  Ar, Ag, Ab,  Bx, By, Bz,  Br, Bg, Bb ]
					use          stride        use          stride
				In this case, the stride would be 3 because we need to skip 3 values (the color values) to reach the next vertex data.
			Apparently the last parameter is the offset? */
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * 5, 0); // vertex pointer
			glEnableVertexAttribArray(0); // Enabling location in VAO for the attribute
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * 5, (void*)(4 * 3)); // uv coord pointer
			glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO

		// Index Buffer Object
		glGenBuffers(1, &id_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * num_of_indices, indices, GL_STATIC_DRAW); // 4 bytes (for uint32) * 12 elements in indices
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0); // Unbind the VAO;
}

void Mesh::render_mesh()
{
	if (index_count == 0) // Early out if index_count == 0, nothing to draw
	{
		// TODO log a warning
		return;
	}

	// Bind VAO, bind VBO, draw elements(indexed draw)
	glBindVertexArray(id_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_ibo);
			// Last param could be pointer to indices but no need cuz IBO is already bound
			glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::clear_mesh()
{
	// Clear GPU memory
	if (id_ibo != 0)
	{
		/* glDeleteBuffers deletes the buffer off the GPU memory. */
		glDeleteBuffers(1, &id_ibo);
		id_ibo = 0;
	}
	if (id_vbo != 0)
	{
		/* glDeleteBuffers deletes the buffer off the GPU memory. */
		glDeleteBuffers(1, &id_vbo);
		id_vbo = 0;
	}
	if (id_vao != 0)
	{
		/* glDeleteVertexArrays deletes the Vertex Array Object off the GPU memory. */
		glDeleteVertexArrays(1, &id_vao);
		id_vao = 0;
	}

	// Reset index_count
	index_count = 0;
}