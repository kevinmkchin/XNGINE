#include "camera.h"
#include "timer.h"
#include "../singletons/input_manager.h"
#include "../singletons/render_manager.h"
#include "../modules/console.h"

void camera_t::update_camera()
{
    real32 dt = timer::delta_time;

    int32 g_mouse_delta_x = input_manager::get_instance()->g_mouse_delta_x;
    int32 g_mouse_delta_y = input_manager::get_instance()->g_mouse_delta_y;
    if(abs(g_mouse_delta_x) < 50.f && abs(g_mouse_delta_y) < 50.f) // don't move if mouse delta is too big to be normal
    {
        rotation.y /*yaw*/ -= g_mouse_delta_x * turnspeed;
        rotation.z /*pitch*/ -= g_mouse_delta_y * turnspeed;
    }

    if(rotation.z > 89.f)
    {
        rotation.z = 89.f;
    }
    if(rotation.z < -89.f)
    {
        rotation.z = -89.f;
    }

    // Calcuate direction, right, and up vectors
    calculated_direction = orientation_to_direction(euler_to_quat(rotation*KC_DEG2RAD));
    calculated_direction = normalize(calculated_direction);
    calculated_right = normalize(cross(calculated_direction, world_up)); // right vector is cross product of direction and up direction of world
    calculated_up = normalize(cross(calculated_right, calculated_direction)); // up vector is cross product of right vector and direction

    const uint8* keystate = input_manager::get_instance()->g_keystate;
    if(console_is_hidden())
    {
        // Check Inputs
        if (keystate[SDL_SCANCODE_W])
        {
            position += calculated_direction * movespeed * dt;
        }
        if (keystate[SDL_SCANCODE_A])
        {
            position += -calculated_right * movespeed * dt;
        }
        if (keystate[SDL_SCANCODE_S])
        {
            position += -calculated_direction * movespeed * dt;
        }
        if (keystate[SDL_SCANCODE_D])
        {
            position += calculated_right * movespeed * dt;
        }
        if (keystate[SDL_SCANCODE_Q])
        {
            position.y += -movespeed * dt;
        }
        if (keystate[SDL_SCANCODE_E])
        {
            position.y += movespeed * dt;
        }
    }
}

void camera_t::calculate_perspective_matrix()
{
    real32 fov = 90.f;
    vec2i display_buffer_size = render_manager::get_instance()->get_buffer_size();
    real32 aspect_ratio = (real32)display_buffer_size.x / (real32)display_buffer_size.y;

#if 0
    matrix_perspective = projection_matrix_orthographic(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 100.f);
#else
    matrix_perspective = projection_matrix_perspective((fov/2.f)*KC_DEG2RAD, aspect_ratio, 0.1f, 1000.f);
#endif
}

/** Returns a view matrix using the given camera as the observer */
void camera_t::calculate_view_matrix()
{
    matrix_view = view_matrix_look_at(position,position + calculated_direction, calculated_up);
}