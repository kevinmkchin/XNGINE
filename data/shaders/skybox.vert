#version 430 core

layout (location = 0) in vec3 pos;

out vec3 tex_coords;

uniform mat4 matrix_view;
uniform mat4 matrix_proj_perspective;

void main()
{
    tex_coords = pos;
    gl_Position = matrix_proj_perspective * matrix_view * vec4(pos, 1.0);
}