#pragma once

#include "../gamedefine.h"
#include "../core/kc_math.h"

/** camera_t properties */
struct camera_t
{
    vec3   position             = { 0.f };            // camera x y z pos in world space
    vec3   rotation             = { 0.f };            // roll yaw pitch - in that order
    vec3   world_up             = { 0.f, 1.f, 0.f };

    vec3   calculated_direction = { 0.f };            // Intuitive direction - direction forward
    vec3   calculated_up        = { 0.f };
    vec3   calculated_right     = { 0.f };

    mat4   matrix_perspective   = { 0.f }; // Perspective projection matrix
    mat4   matrix_view          = { 0.f }; // Last calculated view matrix

    float movespeed             = 12.f;
    float turnspeed             = 0.17f;

    void update_camera();

    void calculate_perspective_matrix();

    void calculate_view_matrix();
};
