/*

Name ideas:
kon
console
konsol
consol


Single header library
Console backend for parsing and interpreting commands from an input stream.

Interprets with side effects then outputs 

Bind Variables and Functions during runtime. Once bound, these variables 
can be mutated and these functions can be called all via commands from an
input stream. e.g. std::cin or an input stream from a visual console GUI.

Leveraging the power of lambdas
Uses function signature that takes input stream and output stream
We can create lambdas of this function signature

CVar https://en.wikipedia.org/wiki/CVAR
Cmd  commands

Built in commands:



*/

#pragma once

#include <iostream>
#include <sstream>
#include <functional>
#include <unordered_map>

struct con
{
    con()
    {
        bind_builtin_commands();
    }

    typedef std::function<void(std::istream& is, std::ostream& os)> console_function_t;
    typedef std::unordered_map<std::string, console_function_t> function_table_t;

    function_table_t cmd_table;
    function_table_t cvar_setter_lambdas;
    function_table_t cvar_getter_lambdas;

    template<class T>
    void bind_cvar(const std::string& vid, T* var_address)
    {
        cvar_setter_lambdas[vid] =
            [vid, var_address](std::istream& is, std::ostream& os)
            {
                T read;
                is >> read;

                if(is.fail())
                {
                    const char* vt = typeid(T).name();
                    os << "CONSOLE ERROR: Type mismatch. CVar '" << vid 
                    << "' is of type '" << vt << "'." << std::endl;

                    is.clear();
                    is.ignore();
                }
                else
                {
                    *var_address = read;
                }
            };

        cvar_getter_lambdas[vid] =
            [var_address](std::istream& is, std::ostream& os)
            {
                os << *var_address << std::endl;
            };
    }

    //void unbind_cvar();

    // void bind_cmd()
    // {

    // }

    // void bind_member_cmd()
    // {

    // }

    void execute(std::istream& input, std::ostream& output)
    {
        std::string cmd_id;
        input >> cmd_id; 
        if(input.fail())
        {
            output << "CONSOLE ERROR: Input isn't a command." << std::endl;
            input.clear();
            input.ignore();
            return;
        }

        function_table_t::iterator cmd_iter = cmd_table.find(cmd_id);
        if(cmd_iter == cmd_table.end())
        {
            output << "CONSOLE ERROR: Input isn't a command." << std::endl;
            input.clear();
            input.ignore();
            return;
        }

        // find cmd_id in cmd map and call execute helper with cmd_id and input stream
        (cmd_iter->second)(input, output);
    }

    void execute(const std::string& str, std::ostream& output)
    {
        std::stringstream line_stream;
        line_stream.str(str);
        execute(line_stream, output);
    }

private:
    void bind_builtin_commands()
    {
        cmd_table["set"] = 
            [this](std::istream& is, std::ostream& os)
            {
                std::string vid;
                is >> vid;
                function_table_t::iterator v_iter = cvar_setter_lambdas.find(vid);
                if(v_iter == cvar_setter_lambdas.end())
                {
                    os << "CONSOLE ERROR: There is no bound variable with id ''." << vid << std::endl;
                    return;
                }
                else
                {
                    (v_iter->second)(is, os);
                }
            };

        cmd_table["get"] =
            [this](std::istream& is, std::ostream& os)
            {
                std::string vid;
                is >> vid;
                function_table_t::iterator v_iter = cvar_getter_lambdas.find(vid);
                if(v_iter == cvar_getter_lambdas.end())
                {
                    os << "CONSOLE ERROR: There is no bound variable with id ''." << vid << std::endl;
                    return;
                }
                else
                {
                    (v_iter->second)(is, os);
                }
            };
    }
};