#pragma once

#include "../gamedefine.h"

struct game_state;

struct input_manager {

    void initialize();

    void process_events();

    game_state* gs = nullptr;

    const uint8* g_keystate = nullptr; // Stores keyboard state this frame. Access via g_keystate[SDL_Scancode].
    int32 g_last_mouse_pos_x = INDEX_NONE; // Stores mouse state this frame. mouse_pos is not updated when using SDL RelativeMouseMode.
    int32 g_last_mouse_pos_y = INDEX_NONE;
    int32 g_curr_mouse_pos_x = INDEX_NONE;
    int32 g_curr_mouse_pos_y = INDEX_NONE;
    int32 g_mouse_delta_x = INDEX_NONE;
    int32 g_mouse_delta_y = INDEX_NONE;

    SINGLETON(input_manager)
};
