/**  */
INTERNAL void update_camera(Camera& camera, real32 dt)
{
	// Check Mouse
	camera.rotation.y += g_mouse_delta_x * camera.turnspeed;
	camera.rotation.x += -g_mouse_delta_y * camera.turnspeed;

	if(camera.rotation.x > 89.f)
	{
		camera.rotation.x = 89.f;
	}
	if(camera.rotation.x < -89.f)
	{
		camera.rotation.x = -89.f;
	}

	// Calcuate direction, right, and up vectors
	camera.calculated_direction.x = cos(camera.rotation.y * TO_RADIANS) * cos(camera.rotation.x * TO_RADIANS);
	camera.calculated_direction.y = sin(camera.rotation.x * TO_RADIANS);
	camera.calculated_direction.z = sin(camera.rotation.y * TO_RADIANS) * cos(camera.rotation.x * TO_RADIANS);
	camera.calculated_direction = glm::normalize(camera.calculated_direction);
	camera.calculated_right = glm::normalize(glm::cross(camera.calculated_direction, camera.world_up)); // right vector is cross product of direction and up direction of world
	camera.calculated_up = glm::normalize(glm::cross(camera.calculated_right, camera.calculated_direction)); // up vector is cross product of right vector and direction

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

/**  */
INTERNAL glm::mat4 calculate_viewmatrix(Camera& camera)
{
	return glm::lookAt(camera.position,
					   camera.position + camera.calculated_direction,
					   camera.calculated_up);
}