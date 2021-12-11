#include "game_statics.h"
#include "core/input.h"
#include "core/display.h"
#include "renderer/deferred_renderer.h"
#include "game/game_state.h"


display* game_statics::the_display = nullptr;
input* game_statics::the_input = nullptr;
deferred_renderer* game_statics::the_renderer = nullptr;
game_state* game_statics::gameState = nullptr;