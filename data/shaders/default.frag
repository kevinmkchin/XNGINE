#version 330 core

in vec2 tex_coord;

uniform sampler2D texture_sampler_0;

out vec4 colour;

void main()
{
    colour = texture(texture_sampler_0, tex_coord);
}