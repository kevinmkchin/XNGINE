#include "game_state.h"
#include "../debugging/debug_drawer.h"
#include "../debugging/console.h"

game_state::game_state()
{
    get_console().bind_cmd("camstats", [this](std::istream& is, std::ostream& os){
        console_printf("camera position x: %f, y: %f, z: %f \n", m_camera.position.x, m_camera.position.y, m_camera.position.z);
        console_printf("camera rotation roll: %f, yaw: %f, pitch: %f \n", m_camera.rotation.x, m_camera.rotation.y, m_camera.rotation.z);
        console_printf("camera speed: %f\n", m_camera.movespeed);
        console_printf("camera sensitivity: %f\n", m_camera.turnspeed);
    });

    get_console().bind_cvar("camspeed", &m_camera.movespeed);
    get_console().bind_cvar("sensitivity", &m_camera.turnspeed);
}

game_state::~game_state()
{
    get_console().unbind_cmd("camstats");
    get_console().unbind_cvar("camspeed");
    get_console().unbind_cvar("sensitivity");
}

void game_state::temp_initialize_Sponza_Pointlight()
{
    directionallight.orientation = euler_to_quat(make_vec3(0.f, 30.f, -47.f) * KC_DEG2RAD);
    directionallight.ambient_intensity = 0.2f; //0.01f;
    directionallight.diffuse_intensity = 0.0f;
    directionallight.colour = { 1.f, 1.f, 1.f };

    auto temp_model_MEM_LEAK_lol = new mesh_group_t();
    *temp_model_MEM_LEAK_lol = mesh_group_t::assimp_load("data/models/sponza/sponza.obj");

    auto parent_obj_MEM_LEAK = new game_object();
    parent_obj_MEM_LEAK->set_render_model(temp_model_MEM_LEAK_lol);
    parent_obj_MEM_LEAK->pos = make_vec3(0.f, -6.f, 0.f);
    parent_obj_MEM_LEAK->scale = make_vec3(0.04f, 0.04f, 0.04f);

    auto child_obj_MEM_LEAK = new game_object();
    child_obj_MEM_LEAK->set_render_model(temp_model_MEM_LEAK_lol);
    child_obj_MEM_LEAK->pos = make_vec3(3300.f, -60.f, 0.f);

    parent_obj_MEM_LEAK->add_child(child_obj_MEM_LEAK);
    add_object_to_scene(parent_obj_MEM_LEAK);

    update_group_1.push_back(parent_obj_MEM_LEAK);
    update_group_1.push_back(child_obj_MEM_LEAK);

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

void game_state::update_scene()
{
    m_camera.update_camera();

    for(auto& game_object_ptr : update_group_1)
    {
        game_object_ptr->update();
    }

    /*
    for(auto& game_object_ptr : update_group_2)
    {
        game_object->update();
    }
    ...
    */
}

void game_state::render_scene(shader_t *render_shader)
{
    // TODO Kevin specific order of rendering might be important sometimes
    mat4 scene_model_matrix = identity_mat4();
    scene_root_object.render(render_shader, &scene_model_matrix);
}

void game_state::switch_map(const char* map_file_path)
{
    console_printf("WARNING: UNIMPLEMENTED");
    console_printf("Loading map: %s\n", map_file_path);
}

void game_state::add_object_to_scene(game_object* render_object)
{
    scene_root_object.add_child(render_object);
}

void game_state::remove_object_from_scene(game_object* render_object)
{
    scene_root_object.remove_child(render_object);
}
