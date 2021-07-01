#pragma once

#include "../gamedefine.h"
#include "../kc_math.h"

struct light_t
{
    vec3    colour = { 1.f, 1.f, 1.f };
    real32  ambient_intensity = 0.2f;
    real32  diffuse_intensity = 1.0f;
};

struct directional_light_t : light_t
{
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };
};

struct point_light_t : light_t
{
    vec3        position = { 0.f, 0.f, 0.f };
    // Attenuation coefficients
    real32      att_constant = 0.3f;
    real32      att_linear = 0.2f;
    real32      att_quadratic = 0.1f;
};

struct spot_light_t : point_light_t
{
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };

    void set_cutoff_in_degrees(float degrees);
    void set_cutoff_in_radians(float radians);
    real32 cosine_cutoff() { return cos_cutoff; }

private:
    real32 cos_cutoff = 0.866f;
};
