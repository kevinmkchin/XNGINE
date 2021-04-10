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
    console::cprintf("result: %f\n", x + y);
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
    console::cprint("Commands in commmands.cpp\n");
    console::cprint("======\n");
    for(auto const& cmd : con_commands)
    {
        std::string cmd_string = cmd.first;
        console::cprint(" ");
        console::cprint(cmd_string.c_str());
        for(size_t type_hash : cmd.second.arg_types)
        {
            if(type_hash == TYPEHASH(int))
            {
                console::cprint(" int");
            }
            else if(type_hash == TYPEHASH(float))
            {
                console::cprint(" float");
            }
            else if(type_hash == TYPEHASH(std::string))
            {
                console::cprint(" string");
            }
            else
            {
                console::cprint(" unknown_arg_type");
            }
        }
        console::cprint("\n");
    }
    console::cprint("\n======\n");
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

// find entities where x attribute is true or has x attribute

// goto entity

////////////////////////////////////////////////////////////////////////////

internal void REGISTER_CONSOLE_COMMANDS()
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

    ADD_COMMAND_ONEARG("profiler", profiler::set_level, int);
    ADD_COMMAND_ONEARG("debug", debug::set_level, int);
    ADD_COMMAND_NOARG("toggle_debug_pointlights", debug::toggle_debug_pointlights);
    ADD_COMMAND_NOARG("togglewireframe", cmd_wireframe);
}