#pragma once

// TODO THIS IS ALL FUCKING TEMPORARY

#include <vector>
#include "../core/kc_math.h"
#include "../renderer/light.h"
#include "../renderer/mesh_group.h"
#include "../renderer/camera.h"

struct gameobject_t
{
    // TODO this is not very data oriented of you
    vec3        pos = {0.f};
    quaternion  orient = identity_quaternion();
    vec3        scale = {1.f,1.f,1.f};
    mesh_group_t  model;
    // int32 flags
    // Tags tags[4]; // primary, secondary, tertiary, quaternary tags
};


struct temp_map_t
{
    // temporary
    gameobject_t mainobject;
    vec3 cam_start_pos = {0.f};
    vec3 cam_start_rot = {0.f};

    // prob going to stay
    std::vector<point_light_t> pointlights;
    std::vector<spot_light_t> spotlights;
    directional_light_t directionallight;
};

struct game_state
{
    void temp_initialize();

    void update();

    bool b_is_game_running = true;
    bool b_is_update_running = true;

    camera_t m_camera;
    temp_map_t loaded_map;
};


