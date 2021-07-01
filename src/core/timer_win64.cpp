#include "timer.h"
float timer::delta_time = -1.f;


/**

    Windows x64 Implementation of timer

    QueryPerformanceCounter has a precision in the microsecond to nanosecond range.
    GetTickCount have between 4 and 15 millisecond precision on most systems.
    QueryPerformanceCounter also works on Xbox.

*/

#include <Windows.h> // required by profileapi.h
#include <profileapi.h>

i64 timer::counter_frequency()
{
    local_persist i64 frequency = 0;
    if(!frequency)
    {
        LARGE_INTEGER perf_counter_frequency_result;
        QueryPerformanceFrequency(&perf_counter_frequency_result);
        frequency = perf_counter_frequency_result.QuadPart;
    }
    return frequency;
}

i64 timer::get_ticks()
{
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
    {
        return -1;
    }
    return ticks.QuadPart;
}

float timer::timestamp()
{
    local_persist i64 last_tick = timer::get_ticks();
    i64 this_tick = timer::get_ticks();
    i64 delta_tick = this_tick - last_tick;
    float deltatime_secs = (float) delta_tick / (float) timer::counter_frequency();
    last_tick = this_tick;
    return deltatime_secs;
}
