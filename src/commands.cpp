/**

IN-GAME CONSOLE COMMANDS

*/

/** Initializes a std::vector<std::string> called args with the arguments passed to the command. */
#define CON_ARGS int argc, ...
#define INIT_CON_ARGS \
    std::vector<std::string> args; \
    if(argc > 0) \
    { \
        va_list argptr; \
        va_start(argptr, argc); \
        for(int i = 0; i < argc; ++i) \
        { \
            args.push_back(va_arg(argptr, std::string)); \
        } \
        va_end(argptr); \
    }

typedef void(*con_commandfunc)(int, ...);
std::map<std::string, con_commandfunc> con_commands;

/** 

HOW TO USE:

    1.  Create a function you want a certain command to call. For the 
        For the arguments, type the macro "CON ARGS" like so:

    e.g. static void cmd_somefunction(CON_ARGS)
         {
             printf("Hello world!")
         }

    2.  If you want to be able to access the arguments passed to the command,
        then type the macro "INIT_CON_ARGS" inside the body of the function
        definition like so:

    e.g. static void cmd_somefunction(CON_ARGS)
         {
             INIT_CON_ARGS

             printf("Hello world!")
         }

        This will give you access to a std::vector<std::string> named "args" 
        which contains the arguments passed to the command in the order they
        are passed.

    3.  You can register the command by calling con_add_command. You can call
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
////////////////////////////////////////////////////////////////////////////

INTERNAL void con_register_cmds()
{
    con_add_command("pause", cmd_pause);
    con_add_command("unpause", cmd_unpause);
}