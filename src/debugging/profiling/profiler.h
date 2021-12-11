#pragma once

struct vtxt_font;
struct texture_t;
struct shader_t;

void profiler_set_level(int level);
int profiler_get_level();

void profiler_initialize(vtxt_font* in_perf_font_handle, texture_t in_perf_font_atlas);

void profiler_render(shader_t* ui_shader, shader_t* text_shader);