#pragma once

#include "../gamedefine.h"
#include "../core/kc_math.h"

struct light_t
{
    vec3    colour = { 1.f, 1.f, 1.f };
    float   ambient_intensity = 0.2f;
    float   diffuse_intensity = 1.0f;
};

struct directional_light_t : light_t
{
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };
};

struct point_light_t : light_t
{
    vec3        position = { 0.f, 0.f, 0.f };
    // Attenuation coefficients
    float       att_constant = 0.3f;
    float       att_linear = 0.2f;
    float       att_quadratic = 0.1f;
};

struct spot_light_t : point_light_t
{
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };

    void set_cutoff_in_degrees(float degrees);
    void set_cutoff_in_radians(float radians);
    float cosine_cutoff() { return cos_cutoff; }

private:
    float cos_cutoff = 0.866f;
};
