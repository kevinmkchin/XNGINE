#pragma once

#include "../gamedefine.h"

struct display
{
    void initialize();
    void clean_up();

    void swap_buffers();

    void display_size_changed();

    void vsync(int vsync);

private:
    
    SINGLETON(display);
};