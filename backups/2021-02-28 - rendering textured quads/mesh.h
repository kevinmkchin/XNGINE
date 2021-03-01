#pragma once

class Mesh
{
public:
	Mesh();
	~Mesh();

	// Create mesh
	void create_mesh(float* vertices, uint32* indices, uint32 num_of_vertices, uint32 num_of_indices);
	// Draw mesh to gl context
	void render_mesh();
	// Clears the GPU memory of this mesh's VAOs and buffer objects
	void clear_mesh();

private:
	// Holds the ID for the VAO, VBO, IBO in the gpu memory
	uint32 id_vao, id_vbo, id_ibo;
	int32 index_count;

};
