#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gSpecularAndShininess;

in vec3 frag_pos;
in vec2 tex_coord;
in vec3 normal;

struct Material
{
    float specular_intensity;
    float shininess;
};
uniform Material material;
uniform sampler2D texture_sampler_0;

void main()
{
    gPosition = frag_pos;
    gNormal = normalize(normal);
    gAlbedo = texture(texture_sampler_0, tex_coord).rgb;
    gSpecularAndShininess.r = material.specular_intensity;
    gSpecularAndShininess.g = material.shininess;
}