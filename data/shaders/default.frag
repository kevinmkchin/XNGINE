#version 330 core

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos; // position of fragment in worldspace

out vec4 colour;

struct DirectionalLight
{
    vec3 colour;
    float ambient_intensity;
    vec3 direction;
    float diffuse_intensity;
};

struct Material
{
    float specular_intensity;
    float shininess;
};

uniform sampler2D texture_sampler_0;
uniform DirectionalLight directional_light;
uniform Material material;
uniform vec3 observer_pos; // camera

void main()
{
    vec4 ambient_colour = vec4(directional_light.colour * directional_light.ambient_intensity, 1.0f);

    float diffuse_factor = max(0.f, dot(normalize(normal), normalize(directional_light.direction)));
    vec4 diffuse_colour = vec4(directional_light.colour * directional_light.diffuse_intensity * diffuse_factor, 1.0f);

    vec4 specular_colour = vec4(0.f,0.f,0.f,0.f);
    if(diffuse_factor > 0.f)
    {
        vec3 observer_vec = normalize(observer_pos - frag_pos);
        vec3 reflection_vec = normalize(reflect(directional_light.direction, normalize(normal)));
        float specular_factor = max(0.f, pow(dot(observer_vec, reflection_vec), material.shininess));
        specular_colour = vec4(directional_light.colour * material.specular_intensity * specular_factor, 1.0f);
    }

    colour = texture(texture_sampler_0, tex_coord) * (ambient_colour + diffuse_colour + specular_colour);
}