#version 330 core

in vec2 tex_coord;

uniform sampler2D texture_sampler_0;

out vec4 colour;

void main()
{
	float text_alpha = texture(texture_sampler_0, tex_coord).x; // x because bitmap is passed as a GL_R value only
	if(text_alpha < 0.3)
	{
		discard;
	}
	colour = vec4(text_alpha, text_alpha, text_alpha, text_alpha);
}