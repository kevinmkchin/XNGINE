#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include "modules/console.h"

/**

    CONSOLE COMMANDS REGISTRATION AND INVOCATION MACRO AND FUNCTION DEFINITIONS

    Only the following functions are important:

    ADD_COMMAND_NOARG           := Register command function with no arguments
    ADD_COMMAND_ONEARG          := Register command function with 1 argument
    ADD_COMMAND_TWOARG          := Register command function with 2 arguments       
    ADD_COMMAND_THREEARG        := Register command function with 3 arguments
    ADD_COMMAND_FOURARG         := Register command function with 4 arguments

    COMMAND_INVOKE              := invokes a console command (identified by the console_command_meta_t) using the args list
*/


typedef void(*command_func_ptr)();

struct console_command_meta_t
{
    command_func_ptr    command_func;
    std::vector<size_t> arg_types;      // type_info::hash_code of arguments
};

void console_register_commands();

std::map<std::string, console_command_meta_t> get_con_commands();

void command_invoke(console_command_meta_t cmd_meta, std::vector<std::string> argslist);