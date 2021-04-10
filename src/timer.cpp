namespace timer
{
    #include <profileapi.h>

    /** Retrieves the frequency of the performance counter. The frequency of
        the performance counter is fixed at system boot and is consistent
        across all processors.  */
    internal inline int64 counter_frequency()
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

    /** Retrieves the current value of the performance counter, which is a
        high resolution (<1us) time stamp that can be used for time-interval
        measurements.*/
    internal inline int64 get_ticks()
    {
        // win64 version of get ticks
        LARGE_INTEGER ticks;
        if (!QueryPerformanceCounter(&ticks))
        {
            return -1;
        }
        return ticks.QuadPart;
    }

    /** Returns the time elapsed in seconds since the last timestamp call. */
    internal inline real32 global_timestamp()
    {
        local_persist int64 last_tick = timer::get_ticks();
        int64 this_tick = timer::get_ticks();
        int64 delta_tick = this_tick - last_tick;
        real32 deltatime_secs = (real32) delta_tick / (real32) timer::counter_frequency();
        last_tick = this_tick;
        return deltatime_secs;
    }

    /** function timers */
}