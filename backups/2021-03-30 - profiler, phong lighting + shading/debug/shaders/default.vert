#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_normal;

out vec2 tex_coord;
out vec3 normal;
out vec3 frag_pos;

uniform mat4 matrix_model;
uniform mat4 matrix_view;
uniform mat4 matrix_projection;

void main()
{
    vec4 world_position = matrix_model * vec4(pos, 1.0);
    gl_Position = matrix_projection * matrix_view * matrix_model * vec4(pos, 1.0);
    tex_coord = in_tex_coord;
    // using model matrix to account for normal being affected by rotation and scale
    // making model matrix a mat3 is fine because we can discard translation when thinking bout normals
    normal = mat3(transpose(inverse(matrix_model))) * in_normal;
    frag_pos = (matrix_model * vec4(pos, 1.0)).xyz;
}