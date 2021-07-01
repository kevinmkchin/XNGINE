#include <cstdlib>
#include "light.h"


void spot_light_t::set_cutoff_in_degrees(float degrees)
{
    cos_cutoff = cosf(degrees * KC_DEG2RAD);
}

void spot_light_t::set_cutoff_in_radians(float radians)
{
    cos_cutoff = radians;
}
