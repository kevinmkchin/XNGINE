#version 330 core

out vec4 colour;

uniform vec4 frag_colour = vec4(1.f, 1.f, 1.f, 1.f);

void main()
{
    colour = frag_colour;
}