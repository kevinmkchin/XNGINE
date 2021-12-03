#ifndef NOCLIP_CONSOLE_H
#define NOCLIP_CONSOLE_H

/*

noclip.h (WORK IN PROGRESS)

By Kevin Chin 2021

Single-header C++ library providing a console backend for parsing and
interpreting commands and arguments from an input stream.

Uses C++11 features (e.g. parameter pack, lambdas) and a ton of standard
library goodies: <iostream>, <sstream>, <functional>, <unordered_map> from 
C++11 Standard Library.

noclip::console is essentially a shell for your programs to call functions
or mutate variables based on input during runtime. The idea is to provide 
a very easy interface to a sophisticated console backend (shell) which can 
be used to build things like in-game drop-down consoles for games.
Examples: 
https://developer.valvesoftware.com/wiki/Developer_Console 
https://quake.fandom.com/wiki/Console_(Q1)
noclip::console can be used for any program, not just games. If you want to
be able to change variables or call certain functions based on text input,
noclip::console will be useful.

QUICK INTERFACE:
    function    | example
    ------------|-----------------------------------------
    bind_cvar   | console.bind_cvar("health", &health);
                |
    bind_cmd    | console.bind_cmd("command", someFunction);
                | console.bind_cmd("command", &Object::memberFunc, &objectInstance);
                | console.bind_cmd("command", [](std::istream& is, std::ostream& os){ lambda body });
                |
    unbind_cvar | console.unbind_cvar("health"); // useful if 'health' goes out of scope (i.e. dealloc'ed)
                |
    unbind_cmd  | console.unbind_cvar("command"); // useful if object owning 'command' goes out of scope
                |
    execute     | console.execute(std::cin, std::cout);
                | console.execute("set health 99", std::cout);

CREATING A CONSOLE:
    noclip::console console;

USAGE:

All you need to understand is that there are CONSOLE VARIABLES 
(CVar https://en.wikipedia.org/wiki/CVAR) and CONSOLE COMMANDS (Cmd). 
Bind Variables and Functions to CVars and Cmds during runtime. Once bound, 
these variables can be mutated and these functions can be called all via 
commands from an input stream. e.g. std::cin or an input stream from a 
visual console GUI.

Consider the following:
```
int hp = 100;
console.bind_cvar("health", &hp); // binds 'hp' as a CVar with id 'health'
```
then if we execute the command "set health 75", hp will change to 75.

For input and output, noclip::console uses std::istream and std::ostream. This
makes noclip::console very flexible for any kind of program. As long as there
is an input stream and an output stream, noclip::console will work. You can 
convert a string into a sstream (type of istream) and use that as the input.
You can also grab the output stream and then convert that into a string. 
You can even do something like:
```
while(true)
{
    console.execute(std::cin, std::cout);
}
```

IMPLEMENTATION DETAILS:
By leveraging the power of lambdas (anonymous functions) and templates, 
this library manages to implement a very sophisticated backend in very
few lines of code.

By using lambdas, we can capture the behaviour of commands. We can 
look up the lambda/behaviour associated with a given command id and then
invoke that behaviour.

Another way lambdas are used is to be able to set and get variables
without knowing their type. Instead of storing the memory address of 
a given variable, we instead store the behaviour (using a lambda) of 
mutating that variable given an input. This way, we don't need to know 
about the type of the variable when we want to mutate it - we just need 
to give the stored behaviour an input to read from.

*/

#include <iostream>
#include <sstream>
#include <functional>
#include <unordered_map>

namespace noclip
{
    typedef std::function<void(std::istream& is, std::ostream& os)> console_function_t;

    struct console
    {
        console()
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
                [this, vid, vmem](std::istream& is, std::ostream& os)
                {
                    T read = this->evaluate_argument<T>(is, os);

                    if(is.fail())
                    {
                        const char* vt = typeid(T).name();
                        os << "NOCLIP::CONSOLE ERROR: Type mismatch. CVar '" << vid 
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

        template<typename O, typename ... Args> /* Use :: syntax e.g. bind_cmd("name", &A::f, &a) */
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

            auto cmd_iter = cmd_table.find(cmd_id);
            if(cmd_iter == cmd_table.end())
            {
                output << "NOCLIP::CONSOLE ERROR: Input '" << cmd_id << "' isn't a command." << std::endl;
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
            /* base case */
        }

        template<typename T, typename ... Ts>
        void read_arg(std::istream& is, T& first, Ts &... rest)
        {
            /*  Variadic template that recursively iterates each function 
                argument type. For each arg type, parse the argument and
                set value of first. First is a reference to a parameter 
                of read_args_and_execute. */

            std::ostringstream discard;
            first = evaluate_argument<T>(is, discard);
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
                os << "NOCLIP::CONSOLE ERROR: Incorrect argument types." << std::endl;
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

        template<typename T>
        T evaluate_argument(std::istream& is, std::ostream& os)
        {
            /*  Evaluate argument expressions e.g. set x (+ 3 7) */

            while(isspace(is.peek()))
            {
                is.ignore();
            }

            if(is.peek() == '(')
            {
                is.ignore(); // '('
                const int max_argument_size = 256;
                char argument_buffer[max_argument_size];
                is.get(argument_buffer, max_argument_size, ')');
                is.ignore(); // ')'

                std::ostringstream result;
                execute(std::string(argument_buffer), result);

                T read;
                std::istringstream(result.str()) >> read;
                return read;
            }
            else
            {
                T read;
                is >> read;
                return read;
            }
        }

        void bind_builtin_commands()
        {
            cmd_table["set"] = 
                [this](std::istream& is, std::ostream& os)
                {
                    std::string vid;
                    is >> vid;
                    auto v_iter = cvar_setter_lambdas.find(vid);
                    if(v_iter == cvar_setter_lambdas.end())
                    {
                        os << "NOCLIP::CONSOLE ERROR: There is no bound variable with id ''." << vid << std::endl;
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
                    auto v_iter = cvar_getter_lambdas.find(vid);
                    if(v_iter == cvar_getter_lambdas.end())
                    {
                        os << "NOCLIP::CONSOLE ERROR: There is no bound variable with id ''." << vid << std::endl;
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
                    os << "-- noclip::console help --" << std::endl;
                    os << "Set and get bound variables with" << std::endl;
                    os << "set <cvar id> <value>" << std::endl;
                    os << "get <cvar id>" << std::endl;
                    os << std::endl;
                    os << "Call bound and compiled C++ functions with" << std::endl;
                    os << "<cmd id> <arg 0> <arg 1> ... <arg n>" << std::endl;
                    os << std::endl;
                    os << "Get help" << std::endl;
                    os << "help : outputs noclip::console help" << std::endl;
                    os << "listcvars : outputs info about every bound console variable" << std::endl;
                    os << "listcmd : outputs info about every bound console command" << std::endl;
                    os << std::endl;
                    os << "Perform arithematic and modulo operations" << std::endl;
                    os << "(+, -, *, /, %) <lhs> <rhs>" << std::endl;
                    os << std::endl;
                    os << "You can pass expressions as arguments" << std::endl;
                    os << "+ (- 3 2) (* 4 5)" << std::endl;
                    os << "set x (get y)" << std::endl;
                    os << "-------- end help --------" << std::endl;

                    /*
                    print help about the built in commands

                    general help message on how to use the console
                    print all bound cvars and cmds
                    and their help annoations or messages (if any exist)
                    */
                };

            cmd_table["listcvars"] =
                [](std::istream& is, std::ostream& os)
                {

                };

            cmd_table["listcmd"] =
                [](std::istream& is, std::ostream& os)
                {

                };

            cmd_table["+"] =
                [this](std::istream& is, std::ostream& os)
                {
                    float a = this->evaluate_argument<float>(is, os);
                    float b = this->evaluate_argument<float>(is, os);
                    os << a + b << std::endl;
                };

            cmd_table["-"] =
                [this](std::istream& is, std::ostream& os)
                {
                    float a = this->evaluate_argument<float>(is, os);
                    float b = this->evaluate_argument<float>(is, os);
                    os << a - b << std::endl;
                };

            cmd_table["*"] =
                [this](std::istream& is, std::ostream& os)
                {
                    float a = this->evaluate_argument<float>(is, os);
                    float b = this->evaluate_argument<float>(is, os);
                    os << a * b << std::endl;
                };

            cmd_table["/"] =
                [this](std::istream& is, std::ostream& os)
                {
                    float a = this->evaluate_argument<float>(is, os);
                    float b = this->evaluate_argument<float>(is, os);
                    os << a / b << std::endl;
                };

            cmd_table["%"] =
                [this](std::istream& is, std::ostream& os)
                {
                    int a = this->evaluate_argument<int>(is, os);
                    int b = this->evaluate_argument<int>(is, os);
                    os << a % b << std::endl;
                };
        }
    };
}


/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2021 Kevin Chin
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

#endif //NOCLIP_CONSOLE_H
