#pragma once

#include "../gamedefine.h"

struct quaternion;
struct mesh_t;
struct camera_t;
struct shader_t;
struct point_light_t;
struct spot_light_t;


int debug_drawer_get_level();
void create_circle_vertex_buffer(float* vertices, u32* indices,
                                 u32* vertex_count, u32* indices_count,
                                 float x, float y, float z,
                                 float radius, u8 axis=0);
mesh_t create_circle_mesh(float pos_x, float pos_y, float pos_z, float radius, u8 axis=0);
mesh_t create_cone_mesh(float apex_x, float apex_y, float apex_z,
                        float height, float base_radius);
void debug_render_sphere(shader_t& shader, float x, float y, float z, float radius);
void debug_render_cone(shader_t& shader,
                       float x, float y, float z,
                       float height, float base_radius,
                       quaternion orientation);
void debug_render_line();
float debug_calculate_attenuation_range(float c, float l, float q);
void debug_render_pointlight(shader_t& shader, point_light_t& plight);
void debug_render_spotlight(shader_t& shader, spot_light_t& slight);
void debug_initialize();
void debug_render(shader_t& debug_shader, camera_t camera);
void debug_set_pointlights(point_light_t* point_lights_array, u8 count);
void debug_toggle_debug_pointlights();
void debug_set_spotlights(spot_light_t* spot_lights_array, u8 count);
void debug_toggle_debug_spotlights();
void debug_set_debug_level(int level);