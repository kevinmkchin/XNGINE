#include <cstdlib>
#include "light.h"

internal float ATTENUATION_FACTOR_TO_CALC_RANGE_FOR = 0.005f;

internal float calculate_attenuation_range(float c, float l, float q)
{
    c -= 1.f / ATTENUATION_FACTOR_TO_CALC_RANGE_FOR;
    float discriminant = l * l - 4 * q * c;
    if (discriminant >= 0)
    {
        float root1 = (-l + sqrtf(discriminant)) / (2 * q);
        float root2 = (-l - sqrtf(discriminant)) / (2 * q);
        float att_range = max(root1, root2);
        return att_range;
    }
    else
    {
        return 0.f;
    }
}

float point_light_t::update_radius()
{
    radius = calculate_attenuation_range(att_constant, att_linear, att_quadratic);
    return radius;
}

float point_light_t::get_radius() const
{
    return radius;
}

float point_light_t::get_att_constant() const
{
    return att_constant;
}

void point_light_t::set_att_constant(float att_constant)
{
    point_light_t::att_constant = att_constant;
    update_radius();
}

float point_light_t::get_att_linear() const
{
    return att_linear;
}

void point_light_t::set_att_linear(float att_linear)
{
    point_light_t::att_linear = att_linear;
    update_radius();
}

float point_light_t::get_att_quadratic() const
{
    return att_quadratic;
}

void point_light_t::set_att_quadratic(float att_quadratic)
{
    point_light_t::att_quadratic = att_quadratic;
    update_radius();
}

void spot_light_t::set_cutoff_in_degrees(float degrees)
{
    cos_cutoff = cosf(degrees * KC_DEG2RAD);
}

void spot_light_t::set_cutoff_in_radians(float radians)
{
    cos_cutoff = radians;
}
