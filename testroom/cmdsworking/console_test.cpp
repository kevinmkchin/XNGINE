#include "kc_con.h"

void printstuff()
{
    std::cout << "pog" << std::endl;
    std::cout << "yellow" << std::endl;
    std::cout << "arsenal" << std::endl;
}

struct somestruct
{

};

struct A
{
    float x = 3.1415;

    void f(int i)
    {
        std::cout << "A::f + " << x << std::endl;
    }
};

void funcwithargs(const std::string& str, const int& i)
{
    std::cout << "called it" << str << i << std::endl;
}

int main()
{
    int i;
    float f;
    std::string s;

    con c;
    c.bind_cvar("i", &i);
    c.bind_cvar("f", &f);
    c.bind_cvar("s", &s);
    c.bind_cmd("yolo", printstuff);

    c.bind_cmd("yeah", funcwithargs);

    A a;
    c.bind_cvar("ax", &a.x);
    c.bind_cmd("af", &A::f, &a);

    while(true)
    {
        c.execute(std::cin, std::cout);
    }

    return 0;
}