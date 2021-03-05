#version 330 core

// Shader for Rendering Text on Textured Quads

in vec2 tex_coord;

uniform sampler2D font_atlas_sampler;
uniform vec3 text_colour;

out vec4 colour;

void main()
{
	float text_alpha = texture(font_atlas_sampler, tex_coord).x; // x because bitmap is passed as a GL_R value only
	// Make sure alpha blending is on
	colour = vec4(text_colour, text_alpha);
}