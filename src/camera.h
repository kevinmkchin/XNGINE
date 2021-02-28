#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

class Camera
{
public:
	Camera();
	~Camera();

	void update(float dt);

	glm::mat4 calculate_viewmatrix();

	void set_position(float x, float y, float z);
	glm::vec3 get_position() { return position; }
	void set_rotation(float pitch, float yaw, float roll);
	glm::vec3 get_rotation() { return rotation; }

private:
	glm::vec3 position;		// camera x y z pos in world space 
	glm::vec3 rotation;		// pitch, yaw, roll - in that order
	glm::vec3 world_up;

	glm::vec3 calculated_direction; 	// Intuitive direction - direction forward
	glm::vec3 calculated_up;
	glm::vec3 calculated_right;

	float movespeed;
	float turnspeed;
};
