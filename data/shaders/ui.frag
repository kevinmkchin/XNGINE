#version 330 core

// Shader for UI

in vec2 tex_coord;

out vec4 colour;

uniform sampler2D font_atlas_sampler;
uniform vec3 text_colour;

void main()
{
    colour = vec4(0.1, 0.1, 0.1, 1.0); //texture(font_atlas_sampler, tex_coord);
}