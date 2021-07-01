#include "timer.h"
float timer::delta_time = -1.f;


/**

    Windows x64 Implementation of timer

    QueryPerformanceCounter has a precision in the microsecond to nanosecond range.
    GetTickCount have between 4 and 15 millisecond precision on most systems.
    QueryPerformanceCounter also works on Xbox.

*/

#include <windows.h> // required by profileapi.h
#include <profileapi.h>

int64 timer::counter_frequency()
{
    local_persist int64 frequency = 0;
    if(!frequency)
    {
        LARGE_INTEGER perf_counter_frequency_result;
        QueryPerformanceFrequency(&perf_counter_frequency_result);
        frequency = perf_counter_frequency_result.QuadPart;
    }
    return frequency;
}

int64 timer::get_ticks()
{
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
    {
        return -1;
    }
    return ticks.QuadPart;
}

real32 timer::timestamp()
{
    local_persist int64 last_tick = timer::get_ticks();
    int64 this_tick = timer::get_ticks();
    int64 delta_tick = this_tick - last_tick;
    real32 deltatime_secs = (real32) delta_tick / (real32) timer::counter_frequency();
    last_tick = this_tick;
    return deltatime_secs;
}
