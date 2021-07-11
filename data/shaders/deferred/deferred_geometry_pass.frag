#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;

struct Material
{
    float specular_intensity;
    float shininess;
};
uniform Material material;
uniform sampler2D texture_sampler_0;

void main()
{
    gPosition.rgb = frag_pos;
    gPosition.a = material.specular_intensity;
    gNormal.rgb = normalize(normal);
    gNormal.a = material.shininess;
    gAlbedo.rgb = texture(texture_sampler_0, tex_coord).rgb;
}