#pragma once

#include <vector>
#include "../core/kc_math.h"
#include "../renderer/light.h"
#include "../renderer/mesh_group.h"
#include "../renderer/camera.h"
#include "game_object.h"

struct game_state
{
public:
    bool b_is_game_running = true;
    bool b_is_update_running = true;

    directional_light_t         directionallight;
    std::vector<point_light_t>  pointlights;
    camera_t m_camera;
    vec3 cam_start_pos = {0.f};
    vec3 cam_start_rot = {0.f};

    std::vector<game_object*>    update_group_1;
    // update_group_2
    // update_group_3
    // ...

public:
    void temp_initialize_Sponza_Pointlight();

    void update_scene();

    void render_scene(shader_t* render_shader);

    void switch_map(const char* map_file_path);

public:
    /** Add a game_object to the scene */
    void add_object_to_scene(game_object* render_object);
    /** Remove a game_object from the scene */
    void remove_object_from_scene(game_object* render_object);
private:
    /** Root of Scene or World to render. Scene is a graphics concept.
        When referring to Scene, I am talking about Scene relating to
        rendering. */
    game_object scene_root_object;
};


