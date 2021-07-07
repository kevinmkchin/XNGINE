#version 330 core

uniform sampler2D tiled_deferred_shading_texture;

in vec2 tex_coord;

out vec4 frag_colour;

void main()
{
    frag_colour = texture(tiled_deferred_shading_texture, tex_coord);
}
