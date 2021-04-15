/** Update camera position, rotation, direction, and right, up vectors */
internal void update_camera(Camera& camera, real32 dt)
{
    if(abs(g_mouse_delta_x) < 50.f && abs(g_mouse_delta_y) < 50.f) // don't move if mouse delta is too big to be normal
    {
        camera.rotation.y += g_mouse_delta_x * camera.turnspeed;
        camera.rotation.x += -g_mouse_delta_y * camera.turnspeed;       
    }

    if(camera.rotation.x > 89.f)
    {
        camera.rotation.x = 89.f;
    }
    if(camera.rotation.x < -89.f)
    {
        camera.rotation.x = -89.f;
    }

    // Calcuate direction, right, and up vectors
    camera.calculated_direction.x = cos(camera.rotation.y * KC_DEG2RAD) * cos(camera.rotation.x * KC_DEG2RAD);
    camera.calculated_direction.y = sin(camera.rotation.x * KC_DEG2RAD);
    camera.calculated_direction.z = sin(camera.rotation.y * KC_DEG2RAD) * cos(camera.rotation.x * KC_DEG2RAD);
    camera.calculated_direction = normalize(camera.calculated_direction);
    camera.calculated_right = normalize(cross(camera.calculated_direction, camera.world_up)); // right vector is cross product of direction and up direction of world
    camera.calculated_up = normalize(cross(camera.calculated_right, camera.calculated_direction)); // up vector is cross product of right vector and direction

    if(console::is_hidden())
    {
        // Check Inputs
        if (g_keystate[SDL_SCANCODE_W])
        {
            camera.position += camera.calculated_direction * camera.movespeed * dt;
        }
        if (g_keystate[SDL_SCANCODE_A])
        {
            camera.position += -camera.calculated_right * camera.movespeed * dt;
        }
        if (g_keystate[SDL_SCANCODE_S])
        {
            camera.position += -camera.calculated_direction * camera.movespeed * dt;
        }
        if (g_keystate[SDL_SCANCODE_D])
        {
            camera.position += camera.calculated_right * camera.movespeed * dt;
        }
        if (g_keystate[SDL_SCANCODE_Q])
        {
            camera.position.y += -camera.movespeed * dt;
        }
        if (g_keystate[SDL_SCANCODE_E])
        {
            camera.position.y += camera.movespeed * dt;
        }
    }
}

internal mat4 calculate_perspectivematrix(Camera& camera, real32 fov)
{
    real32 aspect_ratio = (real32)g_buffer_width / (real32)g_buffer_height;
    camera.matrix_perspective = projection_matrix_perspective(fov/2.f, aspect_ratio, 0.1f, 1000.f);
    return camera.matrix_perspective;
}

/** Returns a view matrix using the given camera as the observer */
internal mat4 calculate_viewmatrix(Camera& camera)
{
    camera.matrix_view = view_matrix_look_at(camera.position,
        camera.position + camera.calculated_direction, camera.calculated_up);
    return camera.matrix_view;
}