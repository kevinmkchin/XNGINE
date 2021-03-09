/**

    CONSOLE COMMANDS REGISTRATION AND INVOCATION MACRO AND FUNCTION DEFINITIONS

    Only the following functions are important:

    ADD_COMMAND_NOARG           := Register command function with no arguments
    ADD_COMMAND_ONEARG          := Register command function with 1 argument
    ADD_COMMAND_TWOARG          := Register command function with 2 arguments       
    ADD_COMMAND_THREEARG        := Register command function with 3 arguments
    ADD_COMMAND_FOURARG         := Register command function with 4 arguments

    COMMAND_INVOKE              := invokes a console command (identified by the ConCommandMeta) using the args list
*/

typedef void(*command_func_ptr)();

struct ConCommandMeta
{
    command_func_ptr    command_func;
    std::vector<size_t> arg_types;      // type_info::hash_code of arguments
};

GLOBAL_VAR std::map<std::string, ConCommandMeta> con_commands; // association of console command strings to their actual commands


/** 
    CONSOLE COMMAND REGISTRATION MACROS
*/
#define ADD_COMMAND_NOARG(cmd_str, cmd_func) \
            { \
                ConCommandMeta cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_ONEARG(cmd_str, cmd_func, argtype1) \
            { \
                ConCommandMeta cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_TWOARG(cmd_str, cmd_func, argtype1, argtype2); \
            { \
                ConCommandMeta cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype2).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_THREEARG(cmd_str, cmd_func, argtype1, argtype2, argtype3); \
            { \
                ConCommandMeta cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype2).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype3).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }
#define ADD_COMMAND_FOURARG(cmd_str, cmd_func, argtype1, argtype2, argtype3, argtype4); \
            { \
                ConCommandMeta cmd_meta; \
                cmd_meta.command_func = (command_func_ptr) cmd_func; \
                cmd_meta.arg_types.push_back(typeid(argtype1).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype2).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype3).hash_code()); \
                cmd_meta.arg_types.push_back(typeid(argtype4).hash_code()); \
                con_commands.emplace(cmd_str, cmd_meta); \
            }

/** 
    CONSOLE COMMAND INVOCATION HELPERS

    idk I cannot think of a better way to go about this
*/
template<typename T1>
INTERNAL void cmd_finallyinvoke1args(command_func_ptr funcptr, T1 first)
{
    void(*func)(T1) = (void(*)(T1)) funcptr;
    func(first);
}
template<typename T1, typename T2>
INTERNAL void cmd_finallyinvoke2args(command_func_ptr funcptr, T1 first, T2 second)
{
    void(*func)(T1, T2) = (void(*)(T1, T2)) funcptr;
    func(first, second);
}
template<typename T1, typename T2, typename T3>
INTERNAL void cmd_finallyinvoke3args(command_func_ptr funcptr, T1 first, T2 second, T3 third)
{
    void(*func)(T1, T2, T3) = (void(*)(T1, T2, T3)) funcptr;
    func(first, second, third);
}
template<typename T1, typename T2, typename T3, typename T4>
INTERNAL void cmd_finallyinvoke4args(command_func_ptr funcptr, T1 first, T2 second, T3 third, T4 fourth)
{
    void(*func)(T1, T2, T3, T4) = (void(*)(T1, T2, T3, T4)) funcptr;
    func(first, second, third, fourth);
}
template<typename T1, typename T2, typename T3>
INTERNAL void cmd_invokestage4(ConCommandMeta cmd_meta, std::vector<std::string> argslist, T1 first, T2 second, T3 third)
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
        return;
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[3]))
        {
            float f = (float) atof(argslist[3].c_str());
            cmd_finallyinvoke4args(cmd_meta.command_func, first, second, third, f);
            return;
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[3];
        cmd_finallyinvoke4args(cmd_meta.command_func, first, second, third, str);
        return;
    }
    con_print("Invalid arguments...\n");
}
template<typename T1, typename T2>
INTERNAL void cmd_invokestage3(ConCommandMeta cmd_meta, std::vector<std::string> argslist, T1 first, T2 second)
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
            return;
        }
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[2]))
        {
            float f = (float) atof(argslist[2].c_str());
            cmd_invokestage4(cmd_meta, argslist, first, second, f);
            return;
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[2];
        cmd_invokestage4(cmd_meta, argslist, first, second, str);
        return;
    }
    con_print("Invalid arguments...\n");
}
template<typename T1>
INTERNAL void cmd_invokestage2(ConCommandMeta cmd_meta, std::vector<std::string> argslist, T1 first)
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
            return;
        }
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[1]))
        {
            float f = (float) atof(argslist[1].c_str());
            cmd_invokestage3(cmd_meta, argslist, first, f);
            return;
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[1];
        cmd_invokestage3(cmd_meta, argslist, first, str);
        return;
    }
    con_print("Invalid arguments...\n");
}

INTERNAL void COMMAND_INVOKE(ConCommandMeta cmd_meta, std::vector<std::string> argslist)
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
            return;
        }
    }
    else if(argtype == TYPEHASH(float))
    {
        if(is_number(argslist[0]))
        {
            float f = (float) atof(argslist[0].c_str());
            cmd_invokestage2(cmd_meta, argslist, f);
            return;
        }
    }
    else if(argtype == TYPEHASH(std::string))
    {
        std::string str = argslist[0];
        cmd_invokestage2(cmd_meta, argslist, str);
        return;
    }
    con_print("Invalid arguments...\n");
}