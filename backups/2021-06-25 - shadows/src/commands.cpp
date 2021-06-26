/**

    IN-GAME CONSOLE COMMANDS

    This is where commands go

    You can connect any functions to a console command using the
    macros specified here.


    HOW TO USE:



*/
#include "commands.h"

internal void cmd_pause()
{
    b_is_update_running = false;
}

internal void cmd_unpause()
{
    b_is_update_running = true;
}

internal void cmd_wireframe()
{
    g_b_wireframe = !g_b_wireframe;
}

internal void cmd_add(float x, float y)
{
    console_printf("result: %f\n", x + y);
}

internal void cmd_sensitivity(float sens)
{
    g_camera.turnspeed = sens;
}

internal void cmd_camera_speed(float speed)
{
    g_camera.movespeed = speed;
}

internal void cmd_exit()
{
    b_is_game_running = false;
}

internal void cmd_help()
{
    console_print("Commands in commmands.cpp\n");
    console_print("======\n");
    for(auto const& cmd : con_commands)
    {
        std::string cmd_string = cmd.first;
        console_print(" ");
        console_printf(cmd_string.c_str());
        for(size_t type_hash : cmd.second.arg_types)
        {
            if(type_hash == TYPEHASH(int))
            {
                console_printf(" int");
            }
            else if(type_hash == TYPEHASH(float))
            {
                console_printf(" float");
            }
            else if(type_hash == TYPEHASH(std::string))
            {
                console_printf(" string");
            }
            else
            {
                console_printf(" unknown_arg_type");
            }
        }
        console_print("\n");
    }
    console_print("\n======\n");
}

internal void sdl_vsync(int vsync);

internal void sdl_fullscreen(int fullscreen)
{
    switch(fullscreen)
    {
        case 0:{SDL_SetWindowFullscreen(window, 0);}break;
        case 1:{SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);}break;
        case 2:{SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);}break;
    }
}

internal void sdl_set_window_size(int w, int h)
{
    SDL_SetWindowSize(window, w, h);
}

internal void game_switch_map(int map_index) //TODO move this to gamemode.cpp and replace console command with a cmd_map(std::string mapname)
{
    if(0 <= map_index && map_index < array_count(loaded_maps))
    {
        active_map_index = map_index;
        g_camera.position = loaded_maps[active_map_index].cam_start_pos;
        g_camera.rotation = loaded_maps[active_map_index].cam_start_rot;
        update_camera(g_camera, 0.f);
        debug_set_pointlights(loaded_maps[active_map_index].pointlights.data(),
                              (uint8) loaded_maps[active_map_index].pointlights.size());
        debug_set_spotlights(loaded_maps[active_map_index].spotlights.data(),
                              (uint8) loaded_maps[active_map_index].spotlights.size());
        if(loaded_maps[active_map_index].mainobject.model.meshes.size() <= 0)
        {
            assimp_load_meshgroup(loaded_maps[active_map_index].mainobject.model,
                                  loaded_maps[active_map_index].temp_obj_path);
        }
        console_printf("Loaded scene with model: %s\n", loaded_maps[active_map_index].temp_obj_path);
    }
    else
    {
        console_printf("Map index is out of bounds. Try something between 0 and %d\n", array_count(loaded_maps));
    }
}

internal void cmd_print_camera_properties()
{
    console_printf("camera_t position x: %f, y: %f, z: %f \n", g_camera.position.x, g_camera.position.y,
                   g_camera.position.z);
    console_printf("camera_t rotation roll: %f, yaw: %f, pitch: %f \n", g_camera.rotation.x, g_camera.rotation.y,
                   g_camera.rotation.z);
    console_printf("camera_t speed: %f\n", g_camera.movespeed);
    console_printf("camera_t sensitivity: %f\n", g_camera.turnspeed);
}

// find entities where x attribute is true or has x attribute

// goto entity

////////////////////////////////////////////////////////////////////////////

internal void console_register_commands()
{
    ADD_COMMAND_NOARG("help", cmd_help);
    ADD_COMMAND_NOARG("exit", cmd_exit);
    ADD_COMMAND_NOARG("pause", cmd_pause);
    ADD_COMMAND_NOARG("unpause", cmd_unpause);

    ADD_COMMAND_ONEARG("fullscreen", sdl_fullscreen, int);
    ADD_COMMAND_TWOARG("windowsize", sdl_set_window_size, int, int);
    ADD_COMMAND_ONEARG("vsync", sdl_vsync, int);

    ADD_COMMAND_TWOARG("add", cmd_add, float, float);
    ADD_COMMAND_ONEARG("sensitivity", cmd_sensitivity, float);
    ADD_COMMAND_ONEARG("camspeed", cmd_camera_speed, float);

    ADD_COMMAND_ONEARG("profiler", profiler_set_level, int);
    ADD_COMMAND_ONEARG("debug", debug_set_debug_level, int);
    ADD_COMMAND_NOARG("toggle_debug_pointlights", debug_toggle_debug_pointlights);
    ADD_COMMAND_NOARG("togglewireframe", cmd_wireframe);
    
    ADD_COMMAND_NOARG("camstats", cmd_print_camera_properties);

    ADD_COMMAND_ONEARG("map", game_switch_map, int);
}