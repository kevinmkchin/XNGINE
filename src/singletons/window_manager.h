#pragma once

#include <SDL.h>
#include "../gamedefine.h"

struct window_manager
{
    void initialize();

    void swap_buffers();

    void clean_up();

    void on_window_size_changed();

    void vsync(int vsync);

private:
    SDL_Window* window = nullptr;
    SDL_GLContext opengl_context = nullptr;
    
    SINGLETON(window_manager);
};