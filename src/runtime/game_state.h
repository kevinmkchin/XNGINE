#pragma once

#include <vector>
#include "../core/kc_math.h"
#include "../renderer/light.h"
#include "../renderer/mesh_group.h"
#include "../renderer/camera.h"

struct gameobject_t
{
    vec3        pos = {0.f};
    quaternion  orient = identity_quaternion();
    vec3        scale = {1.f,1.f,1.f};
    mesh_group_t model;
    // int32 flags
    // Tags tags[4];
};

struct game_state
{
    void temp_initialize();

    void update();

    void switch_map(const char* map_file_path);

    bool b_is_game_running = true;
    bool b_is_update_running = true;

    std::vector<gameobject_t>   game_objects;
    directional_light_t         directionallight;
    std::vector<point_light_t>  pointlights;
    camera_t m_camera;
    vec3 cam_start_pos = {0.f};
    vec3 cam_start_rot = {0.f};
};


