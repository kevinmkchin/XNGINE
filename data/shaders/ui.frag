#version 330 core

// Shader for UI

in vec2 tex_coord;

out vec4 colour;

uniform bool b_use_colour = false;
uniform sampler2D texture_sampler_0;
uniform vec4 ui_element_colour;

void main()
{
    if(b_use_colour)
    {
        colour = ui_element_colour;
    }
    else
    {
        colour = texture(texture_sampler_0, tex_coord);
    }
}