#version 330

/** shader for rendering shadow map from the point of view 
    of a directional light 
*/

layout (location = 0) in vec3 pos;

uniform mat4 matrix_model;
uniform mat4 directionalLightTransform; // combination of ortho projection matrix * view matrix

void main()
{
    gl_Position = directionalLightTransform * matrix_model * vec4(pos, 1.0);
}