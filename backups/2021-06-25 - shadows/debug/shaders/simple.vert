#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 matrix_model;
uniform mat4 matrix_view;
uniform mat4 matrix_proj_perspective;

void main()
{
    vec4 world_position = matrix_model * vec4(pos, 1.0);
    gl_Position = matrix_proj_perspective * matrix_view * world_position;
}