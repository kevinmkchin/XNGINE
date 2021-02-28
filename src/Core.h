#pragma once
#include "mkc.h"

// --- global variables declarations --- read https://stackoverflow.com/questions/1433204/how-do-i-use-extern-to-share-variables-between-source-files

extern uint32 g_buffer_width;
extern uint32 g_buffer_height;

// keystate this frame
extern const uint8* g_keystate;		// Stores keyboard state. Access via g_keystate[SDL_Scancode].

// mouse state this frame
extern int32 g_last_mouse_pos_x;	// mouse_pos is not updated when using SDL RelativeMouseMode
extern int32 g_last_mouse_pos_y;
extern int32 g_curr_mouse_pos_x;
extern int32 g_curr_mouse_pos_y;
extern int32 g_mouse_delta_x;
extern int32 g_mouse_delta_y;