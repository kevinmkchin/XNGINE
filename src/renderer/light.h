#pragma once

#include "../game_defines.h"
#include "../core/kc_math.h"

struct directional_light_t
{
    vec3    colour = { 1.f, 1.f, 1.f };
    float   diffuse_intensity = 1.0f;
    float   ambient_intensity = 0.2f;
    quaternion orientation = { 0.7071068f, 0.f, 0.f, 0.7071068f };
};

struct point_light_t
{
    vec3    colour = { 1.f, 1.f, 1.f };
    float   diffuse_intensity = 1.0f;
    vec3    position = { 0.f, 0.f, 0.f };

private:
    float       radius;

    // Attenuation coefficients
    float       att_constant;
    float       att_linear;
    float       att_quadratic;
    bool32      b_static;
    bool32      b_cast_shadow;
    bool32      b_prebaked_shadow; // only if b_static is true
    bool32      b_spotlight;
    bool32      :32;
    vec3        direction = { -1.f, -1.f, 1.f };// { 0.f, -1.f, 0.f };
    float       cos_cutoff = 0.866f;

public:
    point_light_t()
    {
        // https://wiki.ogre3d.org/-Point+Light+Attenuation
//        att_constant = 0.3f;
//        att_linear = 0.2f;
//        att_quadratic = 0.1f;
        att_constant = 1.0f;
        att_linear = 0.007f;
        att_quadratic = 0.0002f;

        radius = update_radius();

        b_static = true;
        b_cast_shadow = false;
        b_prebaked_shadow = false;
        b_spotlight = false;
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

    bool is_b_spotlight() const;

    void set_b_spotlight(bool32 b_spotlight);

    const vec3& get_direction() const;

    void set_direction(const vec3& direction);

    void set_cutoff_in_degrees(float degrees);
    void set_cutoff_in_radians(float radians);
    float cosine_cutoff() const { return cos_cutoff; }

private:
    float update_radius();
};
