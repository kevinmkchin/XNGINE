/**

    IN-GAME CONSOLE COMMANDS

    This is where commands go

    You can connect any functions to a console command using the
    macros specified here.


    HOW TO USE:



*/
#include "commands.h"

INTERNAL void cmd_pause()
{
    b_is_update_running = false;
}

INTERNAL void cmd_unpause()
{
    b_is_update_running = true;
}

INTERNAL void cmd_wireframe()
{
    g_b_wireframe = !g_b_wireframe;
}

INTERNAL void cmd_add(float x, float y)
{
    con_printf("result: %f\n", x + y);
}

INTERNAL void cmd_sensitivity(float sens)
{
    g_camera.turnspeed = sens;
}

INTERNAL void cmd_camera_speed(float speed)
{
    g_camera.movespeed = speed;
}

INTERNAL void cmd_exit()
{
    is_running = false;
}

INTERNAL void cmd_help()
{
    con_print("Commands in commmands.cpp\n");
    con_print("======\n");
    for(auto const& cmd : con_commands)
    {
        std::string cmd_string = cmd.first;
        con_print(" ");
        con_printf(cmd_string.c_str());
        for(size_t type_hash : cmd.second.arg_types)
        {
            if(type_hash == TYPEHASH(int))
            {
                con_printf(" int");
            }
            else if(type_hash == TYPEHASH(float))
            {
                con_printf(" float");
            }
            else if(type_hash == TYPEHASH(std::string))
            {
                con_printf(" string");
            }
            else
            {
                con_printf(" unknown_arg_type");
            }
        }
        con_print("\n");
    }
    con_print("\n======\n");
}

INTERNAL void sdl_vsync(int vsync);

INTERNAL void sdl_fullscreen(int fullscreen)
{
    switch(fullscreen)
    {
        case 0:{SDL_SetWindowFullscreen(window, 0);}break;
        case 1:{SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);}break;
        case 2:{SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);}break;
    }
}

INTERNAL void sdl_set_window_size(int w, int h)
{
    SDL_SetWindowSize(window, w, h);
}

// find entities where x attribute is true or has x attribute

// goto entity

////////////////////////////////////////////////////////////////////////////

INTERNAL void con_register_cmds()
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
    ADD_COMMAND_ONEARG("debug", debugger_set_debug_level, int);
    ADD_COMMAND_NOARG("toggle_debug_pointlights", debugger_toggle_debug_pointlights);
    ADD_COMMAND_NOARG("togglewireframe", cmd_wireframe);
}