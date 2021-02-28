#include "Camera.h"

#include <SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Core.h"

Camera::Camera()
	: position(glm::vec3(0.f))
	, rotation(glm::vec3(0.f))
	, world_up(glm::vec3(0.f, 1.f, 0.f))
	, calculated_direction(glm::vec3(0.f))
	, calculated_up(glm::vec3(0.f))
	, calculated_right(glm::vec3(0.f))
	, movespeed(2.f)
	, turnspeed(0.17f)
{

}

Camera::~Camera()
{

}

void Camera::update(float dt)
{
	// Check Mouse
	rotation.y += g_mouse_delta_x * turnspeed;
	rotation.x += -g_mouse_delta_y * turnspeed;

	if(rotation.x > 89.f)
	{
		rotation.x = 89.f;
	}
	if(rotation.x < -89.f)
	{
		rotation.x = -89.f;
	}

	// Calcuate direction, right, and up vectors
	calculated_direction.x = cos(rotation.y * TO_RADIANS) * cos(rotation.x * TO_RADIANS);
	calculated_direction.y = sin(rotation.x * TO_RADIANS);
	calculated_direction.z = sin(rotation.y * TO_RADIANS) * cos(rotation.x * TO_RADIANS);
	calculated_direction = glm::normalize(calculated_direction);
	calculated_right = glm::normalize(glm::cross(calculated_direction, world_up)); // right vector is cross product of direction and up direction of world
	calculated_up = glm::normalize(glm::cross(calculated_right, calculated_direction)); // up vector is cross product of right vector and direction

	// Check Inputs
	if (g_keystate[SDL_SCANCODE_W])
	{
		position += calculated_direction * movespeed * dt;
	}
	if (g_keystate[SDL_SCANCODE_A])
	{
		position += -calculated_right * movespeed * dt;
	}
	if (g_keystate[SDL_SCANCODE_S])
	{
		position += -calculated_direction * movespeed * dt;
	}
	if (g_keystate[SDL_SCANCODE_D])
	{
		position += calculated_right * movespeed * dt;
	}
	if (g_keystate[SDL_SCANCODE_Q])
	{
		position.y += -movespeed * dt;
	}
	if (g_keystate[SDL_SCANCODE_E])
	{
		position.y += movespeed * dt;
	}
}

glm::mat4 Camera::calculate_viewmatrix()
{
	return glm::lookAt(position, position + calculated_direction, calculated_up);
}

void Camera::set_position(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void Camera::set_rotation(float pitch, float yaw, float roll)
{
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;
}