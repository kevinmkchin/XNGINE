#include "game_state.h"
#include "../debugging/debug_drawer.h"


void game_state::temp_initialize()
{
    loaded_map.directionallight.orientation = euler_to_quat(make_vec3(0.f, 30.f, -47.f) * KC_DEG2RAD);
    loaded_map.directionallight.ambient_intensity = 0.35f;
    loaded_map.directionallight.diffuse_intensity = 0.8f;
    loaded_map.directionallight.colour = { 1.f, 1.f, 1.f };
    loaded_map.mainobject.model.assimp_load("data/models/vokselia_spawn/vokselia_spawn.obj");
    loaded_map.mainobject.pos = make_vec3(0.f, -6.f, 0.f);
    //loaded_map.mainobject.scale = make_vec3(0.04f, 0.04f, 0.04f);
    //loaded_map.mainobject.scale = make_vec3(0.25f, 0.25f, 0.25f);
    loaded_map.mainobject.scale = make_vec3(25.f, 25.f, 25.f);
    loaded_map.cam_start_pos = make_vec3(26.f, 0.f, 0.f);
    loaded_map.cam_start_rot = make_vec3(0.f, 180.f, 0.f);
    m_camera.position = loaded_map.cam_start_pos;
    m_camera.rotation = loaded_map.cam_start_rot;
    m_camera.update_camera();
    point_light_t lm0pl;
//    for(int i = 0; i < 4096; ++i)
//    {
//        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
//        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
//        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
//        float x = (float) (rand() % 500) - 250;
//        float z = (float) (rand() % 250) - 125;
//        float y = (float) 0 + (rand() % 100);
//        lm0pl.diffuse_intensity = 0.8f;
//        lm0pl.colour = { r, g, b };
//        lm0pl.position = { x, y, z };
//        lm0pl.set_b_cast_shadow(false);
//        loaded_map.pointlights.push_back(lm0pl);
//    }

    debug_set_pointlights(loaded_map.pointlights.data(), loaded_map.pointlights.size());
}

void game_state::update()
{
    m_camera.update_camera();
}
