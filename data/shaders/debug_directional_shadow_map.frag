#version 330 core

in vec2 tex_coord;

out vec4 colour;

uniform sampler2D directional_shadow_map;

void main()
{
    float depth_value = texture(directional_shadow_map, tex_coord).r;
    colour = vec4(vec3(depth_value), 1.0);
}