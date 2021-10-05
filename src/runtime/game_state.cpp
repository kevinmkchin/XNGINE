#include "game_state.h"
#include "../debugging/debug_drawer.h"
#include "../debugging/console.h"


void game_state::temp_initialize_Sponza_Pointlight()
{
    directionallight.orientation = euler_to_quat(make_vec3(0.f, 30.f, -47.f) * KC_DEG2RAD);
    directionallight.ambient_intensity = 0.01f;
    directionallight.diffuse_intensity = 0.0f;
    directionallight.colour = { 1.f, 1.f, 1.f };

    game_objects.emplace_back();
    game_objects[0].model = mesh_group_t::assimp_load("data/models/sponza/sponza.obj");
    game_objects[0].pos = make_vec3(0.f, -6.f, 0.f);
    game_objects[0].scale = make_vec3(0.04f, 0.04f, 0.04f);

    cam_start_pos = make_vec3(26.f, 0.f, 0.f);
    cam_start_rot = make_vec3(0.f, 180.f, 0.f);
    m_camera.position = cam_start_pos;
    m_camera.rotation = cam_start_rot;
    m_camera.update_camera();

    point_light_t lm0pl;
    lm0pl.diffuse_intensity = 0.9f;
    lm0pl.position = { 0, 19.f, 0 };
    lm0pl.set_b_cast_shadow(true);
    pointlights.push_back(lm0pl);
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

    debug_set_pointlights(pointlights.data(), pointlights.size());
}

void game_state::update()
{
    m_camera.update_camera();
}

void game_state::switch_map(const char* map_file_path)
{
    console_printf("Loading map: %s\n", map_file_path);
}
