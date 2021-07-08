#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gSpecularAndShininess;

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;
in vec4 directional_light_space_pos;

struct Material
{
    float specular_intensity;
    float shininess;
};
uniform Material material;
uniform sampler2D texture_sampler_0;
uniform sampler2D directional_shadow_map;

float CalcDirectionalShadowFactor()
{
    vec3 projCoords = directional_light_space_pos.xyz / directional_light_space_pos.w;
    projCoords = (projCoords * 0.5) + 0.5;

    float current = projCoords.z;
    float bias = 0.005f;
    float shadow = 0.0;

    vec2 texelSize = 1.0 / textureSize(directional_shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(directional_shadow_map, projCoords.xy + vec2(x,y)*texelSize).r;
            shadow += current - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9;

    if(current > 1.0)
    {
        shadow = 0.0;
    }

    return shadow;
}

void main()
{
    gPosition = frag_pos;
    gNormal = normalize(normal);
    gAlbedo = texture(texture_sampler_0, tex_coord).rgb;
    gSpecularAndShininess.r = material.specular_intensity;
    gSpecularAndShininess.g = material.shininess;
    gSpecularAndShininess.b = CalcDirectionalShadowFactor();
}