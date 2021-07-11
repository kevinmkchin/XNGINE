#pragma once

#include "../gamedefine.h"
#include "../core/kc_math.h"

struct light_t
{
    vec3    colour = { 1.f, 1.f, 1.f };
    float   diffuse_intensity = 1.0f;
};

struct directional_light_t : light_t
{
    float   ambient_intensity = 0.2f;
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };
};

struct point_light_t : light_t
{
    vec3        position = { 0.f, 0.f, 0.f };

private:
    float       radius;
    // Attenuation coefficients
    float       att_constant;
    float       att_linear;
    float       att_quadratic;
    bool        b_static;
    bool        b_cast_shadow;
    bool        b_prebaked_shadow; // only if b_static is true
    bool        : 8;

public:
    point_light_t()
    {
        att_constant = 0.3f;
        att_linear = 0.2f;
        att_quadratic = 0.1f;

        radius = update_radius();

        b_static = true;
        b_cast_shadow = true;
        b_prebaked_shadow = false;
    }

    float get_radius() const;

    float get_att_constant() const;
    void set_att_constant(float att_constant);

    float get_att_linear() const;
    void set_att_linear(float att_linear);

    float get_att_quadratic() const;
    void set_att_quadratic(float att_quadratic);

    bool is_b_static() const;
    void set_b_static(bool b_static);

    bool is_b_cast_shadow() const;
    void set_b_cast_shadow(bool b_cast_shadow);

    bool is_b_prebaked_shadow() const;
    void set_b_prebaked_shadow(bool b_prebaked_shadow);

private:
    float update_radius();
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
