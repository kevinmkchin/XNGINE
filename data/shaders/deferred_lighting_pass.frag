#version 330 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecularAndShininess;

in vec2 tex_coord;

out vec4 frag_colour;

const int MAX_POINT_LIGHTS = 42;
const int MAX_SPOT_LIGHTS = 1;

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

uniform DirectionalLight directional_light;
uniform int point_light_count;
uniform PointLight point_light[MAX_POINT_LIGHTS];
uniform vec3 observer_pos; // camera

vec3 FragPos;
vec3 Normal;
vec3 Diffuse;
float SpecularIntensity;
float Shininess;

vec4 calc_light_by_direction(vec3 colour, float ambient_intensity, float diffuse_intensity, vec3 direction, float shadow)
{
    /** Because normals are pointing AWAY from fragment right now while direction
        is pointing TO the fragment, we need to invert the direction. */
    direction = -normalize(direction);
    vec4 ambient_colour = vec4(colour * ambient_intensity, 1.0f);
    float diffuse_factor = max(0.f, dot(normalize(Normal), direction));
    vec4 diffuse_colour = vec4(colour * diffuse_intensity * diffuse_factor, 1.0f);
    vec4 specular_colour = vec4(0.f,0.f,0.f,0.f);
    if(diffuse_factor > 0.f && diffuse_intensity > 0.f)
    {
        vec3 observer_vec = normalize(observer_pos - FragPos);
        vec3 reflection_vec = normalize(reflect(direction, normalize(Normal)));
        float specular_factor = max(0.f, pow(dot(observer_vec, reflection_vec), Shininess));
        specular_colour = vec4(colour * SpecularIntensity * specular_factor, 1.0f);
        //specular_colour *= diffuse_intensity;
    }
    return(ambient_colour + ((1.0 - shadow) * (diffuse_colour + specular_colour)) );
}

vec4 calc_directional_light()
{
    float shadowFactor = 0.f;
    return calc_light_by_direction(directional_light.colour, directional_light.ambient_intensity,
    directional_light.diffuse_intensity, directional_light.direction, shadowFactor);
}

vec4 calc_point_light(PointLight plight, int shadowMapIndex)
{
    vec3 direction = FragPos - plight.position;
    float shadowFactor = 0.f;
    vec4 plight_colour = calc_light_by_direction(plight.colour,
    plight.ambient_intensity, plight.diffuse_intensity, direction, shadowFactor);
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
        total_colour += calc_point_light(point_light[i], i);
    }
    return total_colour;
}

void main()
{
    FragPos = texture(gPosition, tex_coord).rgb;
    Normal = texture(gNormal, tex_coord).rgb;
    Diffuse = texture(gAlbedo, tex_coord).rgb;
    SpecularIntensity = texture(gSpecularAndShininess, tex_coord).r;
    Shininess = texture(gSpecularAndShininess, tex_coord).g;

    vec4 final_colour_visibility = calc_directional_light() + calc_point_lights();

    frag_colour = vec4(Diffuse, 1.f) * final_colour_visibility;
}
