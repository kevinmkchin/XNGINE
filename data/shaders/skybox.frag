#version 430 core

in vec3 tex_coords;

out vec4 colour;

uniform samplerCube skybox_cubemap;

void main()
{
    gl_FragDepth = 0.9999999f;
    colour = texture(skybox_cubemap, tex_coords);
}