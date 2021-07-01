#include "game_state.h"
#include "../modules/debug_drawer.h"


void game_state::temp_initialize()
{
    loaded_map.directionallight.orientation = euler_to_quat(make_vec3(0.f, 30.f, -47.f) * KC_DEG2RAD);
    loaded_map.directionallight.ambient_intensity = 0.3f;
    loaded_map.directionallight.diffuse_intensity = 0.8f;
    loaded_map.directionallight.colour = { 1.f, 1.f, 1.f };
    loaded_map.mainobject.model.assimp_load("data/models/sponza/sponza.obj");
    loaded_map.mainobject.pos = make_vec3(0.f, -6.f, 0.f);
    //loaded_map.mainobject.orient = euler_to_quat(make_vec3(0.f, 30.f, -47.f)*KC_DEG2RAD);
    loaded_map.mainobject.scale = make_vec3(0.04f, 0.04f, 0.04f);
    loaded_map.cam_start_pos = make_vec3(0.f, 0.f, 0.f);
    loaded_map.cam_start_rot = make_vec3(0.f, 211.f, -28.f);
    m_camera.position = loaded_map.cam_start_pos;
    m_camera.rotation = loaded_map.cam_start_rot;
    m_camera.update_camera();
    point_light_t lm0pl;
    lm0pl.colour = { 0.0f, 1.0f, 0.0f };
    lm0pl.position = { -6.f, -2.0f, 4.0f };
    lm0pl.ambient_intensity = 0.f;
    lm0pl.diffuse_intensity = 1.f;
    loaded_map.pointlights.push_back(lm0pl);
    point_light_t lm1pl;
    lm1pl.colour = { 0.0f, 0.0f, 1.0f };
    lm1pl.position = { 10.f, -2.0f, 4.0f };
    lm1pl.ambient_intensity = 0.f;
    lm1pl.diffuse_intensity = 1.f;
    loaded_map.pointlights.push_back(lm1pl);

    debug_set_pointlights(loaded_map.pointlights.data(),(uint8) loaded_map.pointlights.size());
    debug_set_spotlights(loaded_map.spotlights.data(),(uint8) loaded_map.spotlights.size());
}

void game_state::update()
{
    m_camera.update_camera();
}