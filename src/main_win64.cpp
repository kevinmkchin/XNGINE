/** XNGINE

TODO:
    - Memory management / custom memory allocator / replace all mallocs and callocs
    - Improve Game state and Deferred Renderer relationship (esp w the lights)
    - Multiple demo scenes
    - Primitive polygon meshes and objects - basic Cube, Sphere, Cone, Cuboid, etc.
    ~~~
    - Reference count texture resources
    - Resource manager / load resources asynchronously so the game isn't frozen while loading?
    - kc_truetypeassembler.h
        - clean up - allocate all memory on init and deallocate all memory on clean up
        - documentation to say that one can use translation and scaling matrices with the resulting
          vertices in order to transform them on the screen (e.g. animate the text).
    ~~~
    - CASCADED SHADOW MAPS https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
        - make a test map for CSM. (e.g. field of trees or cubes all with shadows)
    - STATIC and DYNAMIC lights and STATIC and DYNAMIC objects/casters
        - DYNAMIC lights need to be marked dirty to update shaders etc. (don't update_scene shaders with light info every frame)
        - for dynamic objects, render the shadow map on-top of the existing shadow map e.g. add more dark spots

Rules:
     - unit vector (x: 1, y: 0, z: 0), aka positive X, is the "forward" direction for objects
     - direction != orientation
     - orientation ~= rotation
     - vec3(1, 0, 0) rotated by the object's orientation (represented by quaternion) is the object's forward direction
     - if quaternion is used to represent an orientation, then the quaternion represents the rotation from the WORLD FORWARD VECTOR
     - RIGHT HAND RULE for everything
     - size (unless using STL containers) means memory size, count means number of elements

BUILD MODES

    INTERNAL_BUILD:
        0 - Build for public release
        1 - Build for developer only

    SLOW_BUILD:
        0 - No slow code allowed
        1 - Slow code fine

*/
#include "game_defines.h"
#include "core/timer.h"
#include "debugging/console.h"
#include "debugging/profiling/profiler.h"
#include "debugging/debug_drawer.h"
#include "core/display.h"
#include "core/input.h"
#include "renderer/deferred_renderer.h"
#include "game/game_state.h"
#include "core/file_system.h"

#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define VERTEXT_IMPLEMENTATION
#include <vertext.h>
#include "game_statics.h"

// Fonts
vtxt_font g_font_handle_c64;
texture_t g_font_atlas_c64;

inline void win64_load_font(vtxt_font* font_handle,
                            texture_t& font_atlas,
                            const char* font_path,
                            u8 font_size)
{
    binary_file_handle_t fontfile;
    read_file_binary(fontfile, font_path);
        if(fontfile.memory)
        {
            vtxt_init_font(font_handle, (u8*) fontfile.memory, font_size);
        }
    free_file_binary(fontfile);
    texture_t::gl_create_from_bitmap(font_atlas,
                                     font_handle->font_atlas.pixels,
                                     font_handle->font_atlas.width,
                                     font_handle->font_atlas.height,
                                     GL_RED, GL_RED);
    free(font_handle->font_atlas.pixels);
}

int main(int argc, char* argv[]) // Our main entry point MUST be in this form when using SDL
{
    game_state i_game_state;

    game_statics::the_renderer = new deferred_renderer();
    game_statics::the_input = new input();
    game_statics::the_display = new display();
    game_statics::gameState = &i_game_state;

    game_statics::the_renderer->gs = &i_game_state;
    game_statics::the_input->gs = &i_game_state;

    game_statics::the_display->initialize(); // e.g. Qt, SDL
    game_statics::the_renderer->initialize(); // OpenGL
    game_statics::the_input->initialize(); // e.g. Qt, SDL

    stbi_set_flip_vertically_on_load(true);
    vtxt_setflags(VTXT_CREATE_INDEX_BUFFER);
    win64_load_font(&g_font_handle_c64, g_font_atlas_c64, "data/fonts/SourceCodePro.ttf", 20); //CONSOLE_TEXT_SIZE
    console_initialize(&g_font_handle_c64, g_font_atlas_c64);
    profiler_initialize(&g_font_handle_c64, g_font_atlas_c64);
    debug_initialize();

    game_statics::the_renderer->load_shaders();

    i_game_state.temp_initialize_Sponza_Pointlight();
    game_statics::the_renderer->temp_create_shadow_maps();
    game_statics::the_renderer->temp_create_geometry_buffer();

    // Game Loop
    i64 perf_counter_frequency = timer::counter_frequency();
    i64 last_tick = timer::get_ticks(); // cpu cycles count of last tick
    while (i_game_state.b_is_game_running)
    {
        game_statics::the_input->process_events();

        if (i_game_state.b_is_game_running == false) { break; }
        i64 this_tick = timer::get_ticks();
        i64 delta_tick = this_tick - last_tick;
        float deltatime_secs = (float) delta_tick / (float) perf_counter_frequency;
        last_tick = this_tick;
        timer::delta_time = deltatime_secs;

        console_update();
        if(i_game_state.b_is_update_running)
        {
            i_game_state.update_scene();
        }

        game_statics::the_renderer->render();
        game_statics::the_display->swap_buffers();
    }

    game_statics::the_renderer->clean_up();
    game_statics::the_display->clean_up();

    return 0;
}