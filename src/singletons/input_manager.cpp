#include "input_manager.h"
#include <SDL.h>
#include "../runtime/game_state.h"
#include "../modules/console.h"
#include "../modules/profiler.h"
#include "window_manager.h"
#include "../modules/debug_drawer.h"

SINGLETON_INIT(input_manager)

void input_manager::initialize()
{
    SDL_SetRelativeMouseMode(SDL_TRUE);         // Lock mouse to window
    g_keystate = SDL_GetKeyboardState(nullptr); // Grab keystate array

}

void input_manager::process_events()
{
    // Store Mouse state
    SDL_bool b_relative_mouse = SDL_GetRelativeMouseMode();
    if(b_relative_mouse)
    {
        SDL_GetRelativeMouseState(&g_mouse_delta_x, &g_mouse_delta_y);
    }
    else
    {
        g_last_mouse_pos_x = g_curr_mouse_pos_x;
        g_last_mouse_pos_y = g_curr_mouse_pos_y;
        SDL_GetMouseState(&g_curr_mouse_pos_x, &g_curr_mouse_pos_y);
        if (g_last_mouse_pos_x >= 0) { g_mouse_delta_x = g_curr_mouse_pos_x - g_last_mouse_pos_x; } else { g_mouse_delta_x = 0; }
        if (g_last_mouse_pos_y >= 0) { g_mouse_delta_y = g_curr_mouse_pos_y - g_last_mouse_pos_y; } else { g_mouse_delta_y = 0; }
    }

    // SDL Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            {
                gs->b_is_game_running = false;
            } break;

            case SDL_WINDOWEVENT:
            {
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        window_manager::get_instance()->on_window_size_changed();
                    } break;
                }
            } break;

            case SDL_MOUSEWHEEL:
            {
                if(console_is_shown() && g_curr_mouse_pos_y < 400.f)
                {
                    if(event.wheel.y > 0)
                    {
                        console_scroll_up();
                    }
                    else if(event.wheel.y < 0)
                    {
                        console_scroll_down();
                    }
                }
            } break;

            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    console_toggle();
                    break;
                }

                if (console_is_shown())
                {
                    console_keydown(event.key);
                    break;
                }

                if (event.key.keysym.sym == SDLK_ESCAPE)// && console_is_hidden())
                {
                    gs->b_is_game_running = false;
                    break;
                }

                if (event.key.keysym.sym == SDLK_F1)
                {
                    profiler_get_level() ? console_command("profiler 0") : console_command("profiler 1");
                    break;
                }

                if (event.key.keysym.sym == SDLK_F2)
                {
                    debug_drawer_get_level() ? console_command("debug 0") : console_command("debug 1");
                    break;
                }

                if (event.key.keysym.sym == SDLK_z)
                {
                    SDL_SetRelativeMouseMode((SDL_bool) !b_relative_mouse);
                    console_printf("mouse grab = %s\n", !b_relative_mouse ? "true" : "false");
                    break;
                }
            } break;
        }
    }
}
