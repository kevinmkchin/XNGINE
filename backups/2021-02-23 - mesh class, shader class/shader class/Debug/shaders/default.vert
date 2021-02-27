#version 330 core
layout (location = 0) in vec3 pos;

out vec4 vertex_colour;

uniform mat4 matrix_model;
uniform mat4 matrix_projection;

void main()
{
	gl_Position = matrix_projection * matrix_model * vec4(pos, 1.0);
	vertex_colour = vec4(clamp(pos, 0.f, 1.f), 1.f);
}