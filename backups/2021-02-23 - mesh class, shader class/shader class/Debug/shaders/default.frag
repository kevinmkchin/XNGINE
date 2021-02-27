#version 330 core

in vec4 vertex_colour;

out vec4 colour;

void main()
{
	colour = vertex_colour;
}