#version 330 core

// Shader for Rendering Text on Textured Quads

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 in_tex_coord;

uniform mat4 matrix_model;
uniform mat4 matrix_view;
uniform mat4 matrix_projection;

out vec2 tex_coord;

void main()
{
	gl_Position = matrix_projection * matrix_model * vec4(pos, 0.0, 1.0);
	tex_coord = in_tex_coord;
}