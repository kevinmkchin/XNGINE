#version 330 core

in vec2 tex_coord;

out vec4 colour;

uniform sampler2D texture_sampler_0;

void main()
{
    colour = texture(texture_sampler_0, tex_coord);
}