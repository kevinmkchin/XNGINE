#pragma once

#include "../game_defines.h"

struct display
{
    void initialize();
    void clean_up();

    void swap_buffers();

    void fullscreen(int mode);

    void set_window_size(int w, int h);

    void vsync(int vsync);

    void display_size_changed();
};