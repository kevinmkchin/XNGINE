/**

IN-GAME CONSOLE COMMANDS

This is where commands go

*/

/** Initializes a std::vector<std::string> called args with the arguments passed to the command. */
#define CON_ARGS std::vector<std::string> args//int argc, ...

typedef void(*con_commandfunc)(std::vector<std::string>);
std::map<std::string, con_commandfunc> con_commands;

/** 

HOW TO USE:

    1.  Create a function you want a certain command to call. For the 
        function parameters, type the macro "CON ARGS" like so:

    e.g. static void cmd_somefunction(CON_ARGS)
         {
             printf("Hello world!")
         }

        CON_ARGS will provide the arguments passed to the command,
        as a vector of strings named "args".

    2.  You can register the command by calling con_add_command. You can call
        con_add_command from within con_register_cmds(). Pass a string
        representation of the command for the first parameter, and pass a
        function pointer to the command function you created in 1. for the
        second parameter.

    e.g. con_add_command("somecommand", cmd_somefunction);

*/
INTERNAL void con_add_command(const char* command, con_commandfunc function)
{
    std::string command_str(command);
    con_commands.emplace(command_str, function);
}

////////////////////////////////////////////////////////////////////////////
INTERNAL void cmd_pause(CON_ARGS)
{
    b_is_update_running = false;
}

INTERNAL void cmd_unpause(CON_ARGS)
{
    b_is_update_running = true;
}

INTERNAL void cmd_wireframe(CON_ARGS)
{
    g_b_wireframe = !g_b_wireframe;
}

INTERNAL void cmd_printargs(CON_ARGS)
{
    for(std::string str : args)
    {
        str += "\n";
        con_print(str.c_str());
    }
}

// find entities where x attribute is true or has x attribute

// goto entity

////////////////////////////////////////////////////////////////////////////

INTERNAL void con_register_cmds()
{
    con_add_command("printargs", cmd_printargs);
    con_add_command("pause", cmd_pause);
    con_add_command("unpause", cmd_unpause);
    con_add_command("togglewireframe", cmd_wireframe);
}