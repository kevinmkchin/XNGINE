/** OpenGL 3D Renderer

TODO:
    - Spot lights for Deferred Renderer
    - Omni-directional shadows for Deferred renderer (only allow up to a set number per scene/view)
    ~~~
    - Skybox
    - BUG console command bug - commands get cut off when entered - could be a memory bug?
    - kc_truetypeassembler.h
        - clean up - allocate all memory on init and deallocate all memory on clean up
        - documentation to say that one can use translation and scaling matrices with the resulting
          vertices in order to transform them on the screen (e.g. animate the text).
    ~~~
    - STATIC and DYNAMIC lights and STATIC and DYNAMIC objects/casters
        - DYNAMIC lights need to be marked dirty to update shaders etc. (don't update shaders with light info every frame)
        - for dynamic objects, render the shadow map on-top of the existing shadow map e.g. add more dark spots
    - CASCADED SHADOW MAPS https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
        - make a test map for CSM. (e.g. field of trees or cubes all with shadows)

Backlog:
    - Resource manager / load resources asynchronously so the game isn't frozen while loading?
    - Memory management / custom memory allocator / replace all mallocs and callocs
    - Baked shadow maps for static lights?
    - Remove all STL usage
    - Load opengl extensions manually - Ditch GLEW https://apoorvaj.io/loading-opengl-without-glew/
    - Ditch SDL and implement windows, input, file io myself
    - Map Editor:
        - console command 'editor' to enter
        - quits the game inside the gamemode, and simply loads the map into the editor (keep camera in same transformation)
        - use console to load and save
            - 'save folder/path/name.map'
            - 'load folder/path/name.map' load automatically saves current map
            - maybe keep loaded maps in memory? until we explicitly call unload? or there are too many maps loaded?
                - so that we can switch between maps without losing the map data in memory
                    - then we can have a "palette" map with a collection of loaded model objs that we can copy instead of finding on disk
            - use console commands to create geometry?
        - texture blending - store the bitmap in memory, and we can write it to map data or an actual bitmap whatever
            - brush system like in unity/unreal where you paint textures, and behind the scenes we can edit the blend map in memory
    - GJK EPA collision system
    - Arrow rendering for debugging
        - in the future arrow can also be used for translation gizmo
    - add SIMD for kc_math library
    - Fixed timestep? for physics only?
    - texture_t GL_NEAREST option
    - texture_t do something like source engine
        - Build simple polygons and shapes, and the textures get wrapped
          automatically(1 unit in vertices is 1 unit in texture uv)
    - Console:
        - option for some console messages to be displayed to game screen.
        - remember previously entered commands
        - shader hotloading/compiling during runtime - pause all update / render while shaders are being recompiled
        - mouse picking entities

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
#include "gamedefine.h"
#include "core/timer.h"
#include "debugging/console.h"
#include "debugging/profiling/profiler.h"
#include "debugging/debug_drawer.h"
#include "core/display.h"
#include "core/input.h"
#include "renderer/render_manager.h"
#include "runtime/game_state.h"
#include "renderer/texture.h"
#include "core/file_system.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define KC_TRUETYPEASSEMBLER_IMPLEMENTATION
#include "kc_truetypeassembler.h"

// Fonts
tta_font_t g_font_handle_c64;
texture_t g_font_atlas_c64;

inline void win64_load_font(tta_font_t* font_handle,
                            texture_t& font_atlas,
                            const char* font_path,
                            u8 font_size)
{
    binary_file_handle_t fontfile;
    read_file_binary(fontfile, font_path);
        if(fontfile.memory)
        {
            kctta_init_font(font_handle, (u8*) fontfile.memory, font_size);
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

    display* i_window_manager = display::get_instance();
    render_manager* i_render_manager = render_manager::get_instance();
    input* i_input_manager = input::get_instance();
    i_render_manager->gs = &i_game_state;
    i_input_manager->gs = &i_game_state;

    i_window_manager->initialize(); // e.g. Qt, SDL
    i_render_manager->initialize(); // OpenGL
    i_input_manager->initialize(); // e.g. Qt, SDL

    stbi_set_flip_vertically_on_load(true);
    kctta_setflags(KCTTA_CREATE_INDEX_BUFFER);
    win64_load_font(&g_font_handle_c64, g_font_atlas_c64, "data/fonts/SourceCodePro.ttf", 20); //CONSOLE_TEXT_SIZE
    console_initialize(&g_font_handle_c64, g_font_atlas_c64);
    profiler_initialize(&g_font_handle_c64, g_font_atlas_c64);
    debug_initialize();

    i_render_manager->load_shaders();

    i_game_state.temp_initialize();
    i_render_manager->temp_create_shadow_maps();
    i_render_manager->temp_create_geometry_buffer();

    // Game Loop
    i64 perf_counter_frequency = timer::counter_frequency();
    i64 last_tick = timer::get_ticks(); // cpu cycles count of last tick
    while (i_game_state.b_is_game_running)
    {
        i_input_manager->process_events();

        if (i_game_state.b_is_game_running == false) { break; }
        i64 this_tick = timer::get_ticks();
        i64 delta_tick = this_tick - last_tick;
        float deltatime_secs = (float) delta_tick / (float) perf_counter_frequency;
        last_tick = this_tick;
        timer::delta_time = deltatime_secs;

        console_update();
        if(i_game_state.b_is_update_running)
        {
            i_game_state.update();
        }

        i_render_manager->render();
        i_window_manager->swap_buffers();
    }

    i_render_manager->clean_up();
    i_window_manager->clean_up();

    return 0;
}