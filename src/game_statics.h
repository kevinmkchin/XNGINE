#pragma once

struct display;
struct input;
struct deferred_renderer;
struct game_state;

struct game_statics
{
    static display* the_display;
    static input* the_input;
    static deferred_renderer* the_renderer;
    static game_state* gameState;
};

