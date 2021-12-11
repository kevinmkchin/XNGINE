#pragma once

#include "../game_defines.h"

/**
    High resolution timer API.
    Implement this class for each platform as each platform will have a different way of counting time.
*/
struct timer
{
    /** Global accessor for delta time. Could be in whatever unit you want, but I like seconds. */
    static float delta_time;

    /** Retrieves the frequency of the performance counter. The frequency of
        the performance counter is fixed at system boot and is consistent
        across all processors.  */
    static i64 counter_frequency();

    /** Retrieves the current value of the performance counter, which is a
        high resolution (<1us) time stamp that can be used for time-interval
        measurements.*/
    static i64 get_ticks();

    /** Returns the time elapsed in seconds since the last timestamp call. */
    static float timestamp();
};