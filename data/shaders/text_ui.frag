#version 330 core

// Shader for Rendering Text on Textured Quads

in vec2 tex_coord;

uniform sampler2D texture_sampler_0;

out vec4 colour;

void main()
{
	float text_alpha = texture(texture_sampler_0, tex_coord).x; // x because bitmap is passed as a GL_R value only
	// Make sure alpha blending is on
	colour = vec4(1.0, 1.0, 1.0, text_alpha);
}