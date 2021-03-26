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

// find entities where x attribute is true or has x attribute

// goto entity

////////////////////////////////////////////////////////////////////////////

INTERNAL void con_register_cmds()
{
    ADD_COMMAND_NOARG("pause", cmd_pause);
    ADD_COMMAND_NOARG("unpause", cmd_unpause);
    ADD_COMMAND_NOARG("togglewireframe", cmd_wireframe);
    ADD_COMMAND_TWOARG("add", cmd_add, float, float);
    ADD_COMMAND_ONEARG("sensitivity", cmd_sensitivity, float);
    ADD_COMMAND_ONEARG("profiler", cmd_profiler_set_level, int);
}