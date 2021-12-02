#ifndef KC_CON_H
#define KC_CON_H

/*

Name ideas:
kon
console
konsol
consol

Uses C++11 features (e.g. parameter pack, lambdas)
Uses iostream, sstream, functional, unordered_map from C++11 Standard Library


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

#include <iostream>
#include <sstream>
#include <functional>
#include <unordered_map>

typedef std::function<void(std::istream& is, std::ostream& os)> console_function_t;

struct con
{
    con()
    {
        bind_builtin_commands();
    }

    typedef std::unordered_map<std::string, console_function_t> function_table_t;
    function_table_t cmd_table;
    function_table_t cvar_setter_lambdas;
    function_table_t cvar_getter_lambdas;

    template<typename T>
    void bind_cvar(const std::string& vid, T* vmem)
    {
        cvar_setter_lambdas[vid] =
            [vid, vmem](std::istream& is, std::ostream& os)
            {
                T read;
                is >> read;

                if(is.fail())
                {
                    const char* vt = typeid(T).name();
                    os << "CONSOLE ERROR: Type mismatch. CVar '" << vid 
                    << "' is of type '" << vt << "'." << std::endl;

                    is.clear();
                    while(!isspace(is.peek()))
                    {
                        is.ignore();
                    }
                    is.ignore();
                }
                else
                {
                    *vmem = read;
                }
            };

        cvar_getter_lambdas[vid] =
            [vmem](std::istream& is, std::ostream& os)
            {
                os << *vmem << std::endl;
            };
    }

    template<typename ... Args>
    void bind_cmd(const std::string& cid, void(*f_ptr)(Args ...))
    {
        cmd_table[cid] = 
            [this, f_ptr](std::istream& is, std::ostream& os)
            {
                auto std_fp = std::function<void(Args ...)>(f_ptr);
                this->materialize_and_execute<Args ...>(is, os, std_fp);
            };
    }

    /* Use :: syntax e.g. bind_cmd("name", &A::f, &a) */
    template<typename O, typename ... Args>
    void bind_cmd(const std::string& cid, void(O::*f_ptr)(Args ...), O* omem)
    {
        std::function<void(Args...)> std_fp =
            [f_ptr, omem](Args ... args)
            {
                (omem->*f_ptr)(args...); // could use std::mem_fn instead
            };

        cmd_table[cid] =
            [this, std_fp](std::istream &is, std::ostream &os)
            {
                this->materialize_and_execute<Args...>(is, os, std_fp);
            };
    }

    void bind_cmd(const std::string& cid, console_function_t iofunc)
    {
        cmd_table[cid] = iofunc;
    }

    void unbind_cvar(const std::string& vid)
    {
        auto v_set_iter = cvar_setter_lambdas.find(vid);
        if(v_set_iter != cvar_setter_lambdas.end())
        {
            cvar_setter_lambdas.erase(v_set_iter);
        }

        auto v_get_iter = cvar_getter_lambdas.find(vid);
        if(v_get_iter != cvar_getter_lambdas.end())
        {
            cvar_getter_lambdas.erase(v_get_iter);
        }
    }

    void unbind_cmd(const std::string& cid)
    {
        auto cmd_iter = cmd_table.find(cid);
        if(cmd_iter != cmd_table.end())
        {
            cmd_table.erase(cmd_iter);
        }
    }

    void execute(std::istream& input, std::ostream& output)
    {
        std::string cmd_id;
        input >> cmd_id;

        function_table_t::iterator cmd_iter = cmd_table.find(cmd_id);
        if(cmd_iter == cmd_table.end())
        {
            output << "CONSOLE ERROR: Input '" << cmd_id << "' isn't a command." << std::endl;
            return;
        }

        (cmd_iter->second)(input, output);
    }

    void execute(const std::string& str, std::ostream& output)
    {
        std::stringstream line_stream;
        line_stream.str(str);
        execute(line_stream, output);
    }

private:
    void read_arg(std::istream& is)
    {
        // base case
    }

    template<typename T, typename ... Ts>
    void read_arg(std::istream& is, T& first, Ts &... rest)
    {
        is >> first; // first is a reference to a parameter from read_args_and_execute
        read_arg(is, rest ...);
    }

    template <typename ... Args>
    void read_args_and_execute(std::istream& is, std::ostream& os, std::function<void(Args ...)> f_ptr, 
        typename std::remove_const<typename std::remove_reference<Args>::type>::type ... temps)
    {
        read_arg(is, temps...);

        if(is.fail())
        {
            is.clear();
            os << "CONSOLE ERROR: Incorrect argument types." << std::endl;
            return;
        }
        f_ptr(temps...);
    }

    template<typename ... Args>
    void materialize_and_execute(std::istream& is, std::ostream& os, std::function<void(Args ...)> f_ptr)
    {
        read_args_and_execute(is, os, f_ptr, 
            (materialize<typename std::remove_const<typename std::remove_reference<Args>::type>::type>())...);
    }

    template<typename T>
    T materialize()
    {
        /*  This function is just to turn the arbitrary number of 
            types into an arbitrary number of VALUES of those types. 
            It works because we give this function (input) a type T 
            in the form of a template and it returns (output) a 
            default VALUE of type T. You can sort of think of this as 
            MATERIALIZING the types into their respective default values. */
        return T();
    } 

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

        cmd_table["help"] =
            [](std::istream& is, std::ostream& os)
            {
                os << "-- Console Help --" << std::endl;
                /*
                print help about the built in commands

                general help message on how to use the console
                print all bound cvars and cmds
                and their help annoations or messages (if any exist)
                */
            };

        cmd_table["+"] =
            [](std::istream& is, std::ostream& os)
            {
                /*
                Read the next two inputs, sum them, then output the sum
                */
            };
    }
};

#endif //KC_CON_H