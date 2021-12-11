#pragma once

#include <SDL_events.h>
#include <noclip.h>
#include "../game_defines.h"

noclip::console& get_console();

struct texture_t;
struct shader_t;
struct tta_font_t;

void console_initialize(tta_font_t* in_console_font_handle, texture_t in_console_font_atlas);
void console_toggle();
void console_update();
void console_render(shader_t* ui_shader, shader_t* text_shader);

void console_print(const char* message);
void console_printf(const char* fmt, ...);
void console_command(char* text_command);

void console_keydown(SDL_KeyboardEvent& keyevent);
void console_scroll_up();
void console_scroll_down();
bool console_is_shown();
bool console_is_hidden();