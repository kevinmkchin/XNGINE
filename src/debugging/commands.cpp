/**

    IN-GAME CONSOLE COMMANDS

    This is where commands go

    You can connect any functions to a console command using the
    macros specified here.


    HOW TO USE:



*/
#include "commands.h"
#include "../debugging/profiling/profiler.h"
#include "debug_drawer.h"

internal std::map<std::string, console_command_meta_t> con_commands; // association of console command strings to their actual commands

std::map<std::string, console_command_meta_t> get_con_commands()
{
    return con_commands;
}

#pragma region CONSOLE_COMMAND_REGISTRATION_MACROS
#define ADD_COMMAND_NOARG(cmd_str, cmd_func) \
            { \
                console_command_meta_t cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_ONEARG(cmd_str, cmd_func, argtype1) \
            { \
                console_command_meta_t cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_TWOARG(cmd_str, cmd_func, argtype1, argtype2); \
            { \
                console_command_meta_t cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype2).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_THREEARG(cmd_str, cmd_func, argtype1, argtype2, argtype3); \
            { \
                console_command_meta_t cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype2).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype3).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_FOURARG(cmd_str, cmd_func, argtype1, argtype2, argtype3, argtype4); \
            { \
                console_command_meta_t cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype2).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype3).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype4).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }

/**
    CONSOLE COMMAND INVOCATION HELPERS
*/
inline bool is_number(std::string str)
{
    size_t len = str.length();
    for (int i = 0; i < len; ++i)
    {
        if (false == (isdigit(str[i]) || (str[i] == '.' && len != 1) || (str[i] == '-' && i == 0)))
        {
            return false;
        }
    }
    return true;
}

template<typename T1>
void cmd_finallyinvoke1args(command_func_ptr funcptr, T1 first)
{
    void(*func)(T1) = (void(*)(T1)) funcptr;
    func(first);
}
template<typename T1, typename T2>
void cmd_finallyinvoke2args(command_func_ptr funcptr, T1 first, T2 second)
{
    void(*func)(T1, T2) = (void(*)(T1, T2)) funcptr;
    func(first, second);
}
template<typename T1, typename T2, typename T3>
void cmd_finallyinvoke3args(command_func_ptr funcptr, T1 first, T2 second, T3 third)
{
    void(*func)(T1, T2, T3) = (void(*)(T1, T2, T3)) funcptr;
    func(first, second, third);
}
template<typename T1, typename T2, typename T3, typename T4>
void cmd_finallyinvoke4args(command_func_ptr funcptr, T1 first, T2 second, T3 third, T4 fourth)
{
    void(*func)(T1, T2, T3, T4) = (void(*)(T1, T2, T3, T4)) funcptr;
    func(first, second, third, fourth);
}
template<typename T1, typename T2, typename T3>
void cmd_invokestage4(console_command_meta_t cmd_meta, std::vector<std::string> argslist, T1 first, T2 second, T3 third)
{
    if(argslist.size() == 3)
    {
        cmd_finallyinvoke3args(cmd_meta.command_func, first, second, third);
        return;
    }

    size_t argtype = cmd_meta.arg_types[3];
    if(argtype == TYPEHASH(int))
    {
        if(is_number(argslist[3]))
        {
            int i = atoi(argslist[3].c_str());
            cmd_finallyinvoke4args(cmd_meta.command_func, first, second, third, i);
        }
        else
        {
            console_print("Invalid arguments: argument 4 must be an integer.\n");
        }
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[3]))
        {
            float f = (float) atof(argslist[3].c_str());
            cmd_finallyinvoke4args(cmd_meta.command_func, first, second, third, f);
        }
        else
        {
            console_print("Invalid arguments: argument 4 must be a float.\n");
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[3];
        cmd_finallyinvoke4args(cmd_meta.command_func, first, second, third, str);
    }
}
template<typename T1, typename T2>
void cmd_invokestage3(console_command_meta_t cmd_meta, std::vector<std::string> argslist, T1 first, T2 second)
{
    if(argslist.size() == 2)
    {
        cmd_finallyinvoke2args(cmd_meta.command_func, first, second);
        return;
    }

    size_t argtype = cmd_meta.arg_types[2];
    if(argtype == TYPEHASH(int))
    {
        if(is_number(argslist[2]))
        {
            int i = atoi(argslist[2].c_str());
            cmd_invokestage4(cmd_meta, argslist, first, second, i);
        }
        else
        {
            console_print("Invalid arguments: argument 3 must be an integer.\n");
        }
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[2]))
        {
            float f = (float) atof(argslist[2].c_str());
            cmd_invokestage4(cmd_meta, argslist, first, second, f);
        }
        else
        {
            console_print("Invalid arguments: argument 3 must be a float.\n");
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[2];
        cmd_invokestage4(cmd_meta, argslist, first, second, str);
    }
}
template<typename T1>
void cmd_invokestage2(console_command_meta_t cmd_meta, std::vector<std::string> argslist, T1 first)
{
    if(argslist.size() == 1)
    {
        cmd_finallyinvoke1args(cmd_meta.command_func, first);
        return;
    }

    size_t argtype = cmd_meta.arg_types[1];
    if(argtype == TYPEHASH(int))
    {
        if(is_number(argslist[1]))
        {
            int i = atoi(argslist[1].c_str());
            cmd_invokestage3(cmd_meta, argslist, first, i);
        }
        else
        {
            console_print("Invalid arguments: argument 2 must be an integer.\n");
        }
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[1]))
        {
            float f = (float) atof(argslist[1].c_str());
            cmd_invokestage3(cmd_meta, argslist, first, f);
        }
        else
        {
            console_print("Invalid arguments: argument 2 must be a float.\n");
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[1];
        cmd_invokestage3(cmd_meta, argslist, first, str);
    }
}

void command_invoke(console_command_meta_t cmd_meta, std::vector<std::string> argslist)
{
    if(argslist.size() == 0)
    {
        cmd_meta.command_func();
        return;
    }

    size_t argtype = cmd_meta.arg_types[0];
    if(argtype == TYPEHASH(int))
    {
        if(is_number(argslist[0]))
        {
            int i = atoi(argslist[0].c_str());
            cmd_invokestage2(cmd_meta, argslist, i);
        }
        else
        {
            console_print("Invalid arguments: argument 1 must be an integer.\n");
        }
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[0]))
        {
            float f = (float) atof(argslist[0].c_str());
            cmd_invokestage2(cmd_meta, argslist, f);
        }
        else
        {
            console_print("Invalid arguments: argument 1 must be a float.\n");
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[0];
        cmd_invokestage2(cmd_meta, argslist, str);
    }
}
#pragma endregion



// TODO Kevin KEEP A REFERENCE TO ACTIVE GAME STATE


//void cmd_wireframe()
//{
//    g_b_wireframe = !g_b_wireframe;
//}

void cmd_add(float x, float y)
{
    console_printf("result: %f\n", x + y);
}

//void cmd_sensitivity(float sens)
//{
//    g_camera.turnspeed = sens;
//}
//
//void cmd_camera_speed(float speed)
//{
//    g_camera.movespeed = speed;
//}
//

void cmd_help()
{
    console_print("Commands in commands.cpp\n");
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

//void sdl_vsync(int vsync);
//
//void sdl_fullscreen(int fullscreen)
//{
//    switch(fullscreen)
//    {
//        case 0:{SDL_SetWindowFullscreen(window, 0);}break;
//        case 1:{SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);}break;
//        case 2:{SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);}break;
//    }
//}
//
//void sdl_set_window_size(int w, int h)
//{
//    SDL_SetWindowSize(window, w, h);
//}
//
//void game_switch_map(int map_index) //TODO move this to gamemode.cpp and replace console command with a cmd_map(std::string mapname)
//{
//    if(0 <= map_index && map_index < array_count(loaded_maps))
//    {
//        active_map_index = map_index;
//        g_camera.position = loaded_maps[active_map_index].cam_start_pos;
//        g_camera.rotation = loaded_maps[active_map_index].cam_start_rot;
//        update_camera(g_camera, 0.f);
//        debug_set_pointlights(loaded_maps[active_map_index].pointlights.data(),
//                              (uint8) loaded_maps[active_map_index].pointlights.size());
//        debug_set_spotlights(loaded_maps[active_map_index].spotlights.data(),
//                              (uint8) loaded_maps[active_map_index].spotlights.size());
//        if(loaded_maps[active_map_index].mainobject.model.meshes.size() <= 0)
//        {
//            assimp_load_meshgroup(loaded_maps[active_map_index].mainobject.model,
//                                  loaded_maps[active_map_index].temp_obj_path);
//        }
//        console_printf("Loaded scene with model: %s\n", loaded_maps[active_map_index].temp_obj_path);
//    }
//    else
//    {
//        console_printf("Map index is out of bounds. Try something between 0 and %d\n", array_count(loaded_maps));
//    }
//}
//
//void cmd_print_camera_properties()
//{
//    console_printf("camera_t position x: %f, y: %f, z: %f \n", g_camera.position.x, g_camera.position.y,
//                   g_camera.position.z);
//    console_printf("camera_t rotation roll: %f, yaw: %f, pitch: %f \n", g_camera.rotation.x, g_camera.rotation.y,
//                   g_camera.rotation.z);
//    console_printf("camera_t speed: %f\n", g_camera.movespeed);
//    console_printf("camera_t sensitivity: %f\n", g_camera.turnspeed);
//}

// find entities where x attribute is true or has x attribute

// goto entity

////////////////////////////////////////////////////////////////////////////

void console_register_commands()
{
    ADD_COMMAND_NOARG("help", cmd_help);
//
//    ADD_COMMAND_ONEARG("fullscreen", sdl_fullscreen, int);
//    ADD_COMMAND_TWOARG("windowsize", sdl_set_window_size, int, int);
//    ADD_COMMAND_ONEARG("vsync", sdl_vsync, int);

    ADD_COMMAND_TWOARG("add", cmd_add, float, float);
//    ADD_COMMAND_ONEARG("sensitivity", cmd_sensitivity, float);
//    ADD_COMMAND_ONEARG("camspeed", cmd_camera_speed, float);

    ADD_COMMAND_ONEARG("profiler", profiler_set_level, int);
    ADD_COMMAND_ONEARG("debug", debug_set_debug_level, int);
    ADD_COMMAND_NOARG("toggle_debug_pointlights", debug_toggle_debug_pointlights);
//    ADD_COMMAND_NOARG("togglewireframe", cmd_wireframe);
//
//    ADD_COMMAND_NOARG("camstats", cmd_print_camera_properties);
//
//    ADD_COMMAND_ONEARG("map", game_switch_map, int);
}