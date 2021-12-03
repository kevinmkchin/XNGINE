#include <iostream>
#include "kc_con.h"

// float var;
// float* addressofvar = &var;

int main()
{
    // std::string cmd;
    // std::string cvar_id;
    // float i = -1.f;
    // std::cin >> cmd;
    // std::cin >> cvar_id;
    // std::cin >> i;
    // *addressofvar = i;

    // std::cout << var;

    int i;
    float f;
    std::string s;

    con c;
    c.bind_cvar("i", &i);
    c.bind_cvar("f", &f);
    c.bind_cvar("s", &s);

    //std::cin >> i;

    while(true)
    {
        c.execute(std::cin, std::cout);
    }


    return 0;
}