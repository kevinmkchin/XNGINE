#pragma once

#include "../gamedefine.h"

struct quaternion;
struct mesh_t;
struct camera_t;
struct shader_t;
struct point_light_t;
struct spot_light_t;


int debug_drawer_get_level();
void create_circle_vertex_buffer(real32* vertices, uint32* indices,
                                 uint32* vertex_count, uint32* indices_count,
                                 real32 x, real32 y, real32 z,
                                 real32 radius, uint8 axis=0);
mesh_t create_circle_mesh(real32 pos_x, real32 pos_y, real32 pos_z, real32 radius, uint8 axis=0);
mesh_t create_cone_mesh(real32 apex_x, real32 apex_y, real32 apex_z,
                        real32 height, real32 base_radius);
void debug_render_sphere(shader_t& shader, real32 x, real32 y, real32 z, real32 radius);
void debug_render_cone(shader_t& shader,
                       real32 x, real32 y, real32 z,
                       real32 height, real32 base_radius,
                       quaternion orientation);
void debug_render_line();
real32 debug_calculate_attenuation_range(real32 c, real32 l, real32 q);
void debug_render_pointlight(shader_t& shader, point_light_t& plight);
void debug_render_spotlight(shader_t& shader, spot_light_t& slight);
void debug_initialize();
void debug_render(shader_t& debug_shader, camera_t camera);
void debug_set_pointlights(point_light_t* point_lights_array, uint8 count);
void debug_toggle_debug_pointlights();
void debug_set_spotlights(spot_light_t* spot_lights_array, uint8 count);
void debug_toggle_debug_spotlights();
void debug_set_debug_level(int level);