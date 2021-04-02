#version 330 core

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos; // position of fragment in worldspace

out vec4 colour;

const int MAX_POINT_LIGHTS = 10;
const int MAX_SPOT_LIGHTS = 10;

struct DirectionalLight
{
    vec3 colour;
    float ambient_intensity;
    float diffuse_intensity;

    vec3 direction;
};

struct PointLight
{
    vec3 colour;
    float ambient_intensity;
    float diffuse_intensity;

    vec3 position;
    float att_constant;
    float att_linear;
    float att_quadratic;
};

struct SpotLight
{
    PointLight plight;

    vec3 direction;
    float cutoff;
};

struct Material
{
    float specular_intensity;
    float shininess;
};

uniform sampler2D texture_sampler_0;
uniform DirectionalLight directional_light;
uniform int point_light_count;
uniform PointLight point_light[MAX_POINT_LIGHTS];
uniform int spot_light_count;
uniform SpotLight spot_light[MAX_SPOT_LIGHTS];
uniform Material material;
uniform vec3 observer_pos; // camera

vec4 calc_light_by_direction(vec3 colour, float ambient_intensity, float diffuse_intensity, vec3 direction)
{
    direction = normalize(direction);
    vec4 ambient_colour = vec4(colour * ambient_intensity, 1.0f);
    float diffuse_factor = max(0.f, dot(normalize(normal), direction));
    vec4 diffuse_colour = vec4(colour * diffuse_intensity * diffuse_factor, 1.0f);
    vec4 specular_colour = vec4(0.f,0.f,0.f,0.f);
    if(diffuse_factor > 0.f && diffuse_intensity > 0.f)
    {
        vec3 observer_vec = normalize(observer_pos - frag_pos);
        vec3 reflection_vec = normalize(reflect(direction, normalize(normal)));
        float specular_factor = max(0.f, pow(dot(observer_vec, reflection_vec), material.shininess));
        specular_colour = vec4(colour * material.specular_intensity * specular_factor, 1.0f);
        //specular_colour *= diffuse_intensity;
    }
    return(ambient_colour + diffuse_colour + specular_colour);
}

vec4 calc_directional_light()
{
    return calc_light_by_direction(directional_light.colour, directional_light.ambient_intensity, 
        directional_light.diffuse_intensity, directional_light.direction);
}

vec4 calc_point_light(PointLight plight)
{
    vec3 direction = frag_pos - plight.position;
    vec4 plight_colour = calc_light_by_direction(plight.colour, 
        plight.ambient_intensity, plight.diffuse_intensity, direction);
    float distance = length(direction);
    float attenuation = 1.f / (plight.att_constant
        + plight.att_linear * distance
        + plight.att_quadratic * distance * distance);

    return plight_colour * attenuation;
}

vec4 calc_point_lights()
{
    vec4 total_colour = vec4(0.f,0.f,0.f,0.f);
    for(int i = 0; i < point_light_count; ++i)
    {
        total_colour += calc_point_light(point_light[i]);
    }
    return total_colour;
}

vec4 calc_spot_lights()
{
    vec4 total_colour = vec4(0.f,0.f,0.f,0.f);
    for(int i = 0; i < spot_light_count; ++i)
    {
        vec3 to_frag_vector = normalize(frag_pos - spot_light[i].plight.position);
        float cos_angle_to_frag = dot(to_frag_vector, spot_light[i].direction);
        if(cos_angle_to_frag > spot_light[i].cutoff)
        {
            float f = 1 - ((1 - cos_angle_to_frag) / (1 - spot_light[i].cutoff));
            vec4 colour = calc_point_light(spot_light[i].plight);
            total_colour += colour * f;
        }
    }
    return total_colour;
}

void main()
{
    vec4 final_colour_visibility = calc_directional_light() + calc_point_lights() + calc_spot_lights();

    colour = texture(texture_sampler_0, tex_coord) * final_colour_visibility;
}