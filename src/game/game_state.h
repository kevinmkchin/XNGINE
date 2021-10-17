#pragma once

#include <vector>
#include "../core/kc_math.h"
#include "../renderer/light.h"
#include "../renderer/mesh_group.h"
#include "../renderer/camera.h"
#include "game_object.h"

struct game_state
{
    void temp_initialize_Sponza_Pointlight();

    void update_scene();

    void render_scene(shader_t* render_shader);

    void switch_map(const char* map_file_path);

    bool b_is_game_running = true;
    bool b_is_update_running = true;

    std::vector<game_object>    game_objects;
    directional_light_t         directionallight;
    std::vector<point_light_t>  pointlights;
    camera_t m_camera;
    vec3 cam_start_pos = {0.f};
    vec3 cam_start_rot = {0.f};
};


