/** OpenGL 3D Renderer

TODO:
    - Temporary level class to hold different scenes (for showcasing purpose initially)
        - Create diff levels (e.g. sponza, minecraft, sponza in diff lightings)
        - Be able to switch between them (to showcase)
    - include a vc.bat to detect msvc installation and devenv (visual studio)
Backlog:
    - Memory management / custom memory allocator / replace all mallocs and callocs
    - Shadow mapping
    - Lower case everything except hash defines
    - Arrow rendering for debugging
        - in the future arrow can also be used for translation gizmo

    - add SIMD for kc_math library
    - Entity - pos, orientation, scale, mesh, few boolean flags, collider, tags
    - Fixed timestep? for physics only?
    - Face culling
    - Texture GL_NEAREST option
    - Texture do something like source engine
        - Build simple polygons and shapes, and the textures get wrapped
          automatically(1 unit in vertices is 1 unit in texture uv)
    - Console:
        - remember previously entered commands
        - shader hotloading/compiling during runtime - pause all update / render while shaders are being recompiled
        - mouse picking entities

THIS PROJECT IS A SINGLE TRANSLATION UNIT BUILD / UNITY BUILD

BUILD MODES

    INTERNAL_BUILD:
        0 - Build for public release
        1 - Build for developer only

    SLOW_BUILD:
        0 - No slow code allowed
        1 - Slow code fine

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <gl/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define KC_TRUETYPEASSEMBLER_IMPLEMENTATION
#include "kc_truetypeassembler.h"
#define KC_MATH_IMPLEMENTATION
#include "kc_math.h"

#include "gamedefine.h" // defines and typedefs
#include "console.h"
#include "core.h"

// --- global variables  --- note: static variables are initialized to their default values
// Width and Height of writable buffer
global_var uint32 g_buffer_width;
global_var uint32 g_buffer_height;

// Global Input Data
global_var const uint8* g_keystate = nullptr;       // Stores keyboard state this frame. Access via g_keystate[SDL_Scancode].
global_var int32 g_last_mouse_pos_x = INDEX_NONE;   // Stores mouse state this frame. mouse_pos is not updated when using SDL RelativeMouseMode.
global_var int32 g_last_mouse_pos_y = INDEX_NONE;
global_var int32 g_curr_mouse_pos_x = INDEX_NONE;
global_var int32 g_curr_mouse_pos_y = INDEX_NONE;
global_var int32 g_mouse_delta_x = INDEX_NONE;
global_var int32 g_mouse_delta_y = INDEX_NONE;

// Game Window and States
global_var bool b_is_game_running = true;
global_var bool b_is_update_running = true;
global_var SDL_Window* window = nullptr;
global_var SDL_GLContext opengl_context = nullptr;

// -------------------------
// Temporary
global_var Camera g_camera;
global_var mat4 g_matrix_projection_ortho;
global_var bool g_b_wireframe = false;

/* NOTE we're not going to have multiple maps loaded at once later on
   eventually we want to load maps from disk when we switch maps
   The only reason we are keeping an array right now is so we can preset
   the model transforms since we can't read that from disk yet. */
global_var uint8 active_map_index = 0;
global_var temp_map_t loaded_maps[4];

// Fonts
TTAFont g_font_handle_c64;
Texture g_font_atlas_c64;
// -------------------------

#include "timer.cpp"
#include "file.cpp"
#include "opengl.cpp"
#include "camera.cpp"
#include "profiler.cpp"
#include "debugger.cpp"
#include "core.cpp"
#include "commands.cpp"
#include "console.cpp"

LightingShader shader_common;
OrthographicShader shader_text;
OrthographicShader shader_ui;
PerspectiveShader shader_simple;
Material material_shiny = { 4.f, 128.f };
Material material_dull = { 0.5f, 1.f };

static const char* vertex_shader_path = "shaders/default_phong.vert";
static const char* frag_shader_path = "shaders/default_phong.frag";
static const char* ui_vs_path = "shaders/ui.vert";
static const char* ui_fs_path = "shaders/ui.frag";
static const char* text_vs_path = "shaders/text_ui.vert";
static const char* text_fs_path = "shaders/text_ui.frag";
static const char* simple_vs_path = "shaders/simple.vert";
static const char* simple_fs_path = "shaders/simple.frag";

internal inline void win64_load_font(TTAFont* font_handle,
                                        Texture& font_atlas,
                                        const char* font_path,
                                        uint8 font_size)
{
    BinaryFileHandle fontfile;
    file::read_binary(fontfile, font_path);
        if(fontfile.memory)
        {
            kctta_init_font(font_handle, (uint8*) fontfile.memory, font_size);
        }
    file::free_binary(fontfile);
    gl::load_texture_from_bitmap(font_atlas,
                                font_handle->font_atlas.pixels,
                                font_handle->font_atlas.width,
                                font_handle->font_atlas.height,
                                GL_RED, GL_RED);
    free(font_handle->font_atlas.pixels);
}

internal inline void sdl_vsync(int vsync)
{
    /** This makes our Buffer Swap (SDL_GL_SwapWindow) synchronized with the monitor's 
    vertical refresh - basically vsync; 0 = immediate, 1 = vsync, -1 = adaptive vsync. 
    Remark: If application requests adaptive vsync and the system does not support it, 
    this function will fail and return -1. In such a case, you should probably retry 
    the call with 1 for the interval. */
    switch(vsync){ // 0 = immediate (no vsync), 1 = vsync, 2 = adaptive vsync
        case 0:{SDL_GL_SetSwapInterval(0);}break;
        case 1:{SDL_GL_SetSwapInterval(1);}break;
        case 2:{if(SDL_GL_SetSwapInterval(-1)==-1) SDL_GL_SetSwapInterval(1);}break;
        default:{console::cprint("Invalid vsync option; 0 = immediate, 1 = vsync, 2 = adaptive vsync");}break;
    }
}

internal inline void gl_update_viewport_size()
{
    /** Get the size of window's underlying drawable in pixels (for use with glViewport).
    Remark: This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI drawable, i.e. the window was created 
    with SDL_WINDOW_ALLOW_HIGHDPI on a platform with high-DPI support (Apple calls this "Retina"), and not disabled by the 
    SDL_HINT_VIDEO_HIGHDPI_DISABLED hint. */
    SDL_GL_GetDrawableSize(window, (int*)&g_buffer_width, (int*)&g_buffer_height);
    glViewport(0, 0, g_buffer_width, g_buffer_height);
    console::cprintf("Viewport updated - x: %d y: %d\n", g_buffer_width, g_buffer_height);
}

/** Create window, set up OpenGL context, initialize SDL and GLEW */
internal bool game_init()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL failed to initialize.\n");
        return false;
    }

    // OpenGL Context Attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // version major
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3); // version minor
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // core means not backward compatible. not using deprecated code.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // allow forward compatibility
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // double buffering is on by default, but let's just call this anyway
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // depth buffer precision of 24 bits 
    // Setup SDL Window
    if ((window = SDL_CreateWindow(
        "test win",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    )) == nullptr)
    {
        printf("SDL window failed to create.\n");
        return false;
    }

    /** GRABBING WINDOW INFORMATION - https://wiki.libsdl.org/SDL_GetWindowWMInfo
    *   Remarks: You must include SDL_syswm.h for the declaration of SDL_SysWMinfo. The info structure must 
        be initialized with the SDL version, and is then filled in with information about the given window, 
        as shown in the Code Example. */
    SDL_SysWMinfo sys_windows_info;
    SDL_VERSION(&sys_windows_info.version);
    if (SDL_GetWindowWMInfo(window, &sys_windows_info)) 
    {
        const char* subsystem = "an unknown system!";
        switch (sys_windows_info.subsystem) {
            case SDL_SYSWM_UNKNOWN:   break;
            case SDL_SYSWM_WINDOWS:   subsystem = "Microsoft Windows(TM)";  break;
            case SDL_SYSWM_X11:       subsystem = "X Window System";        break;
#if SDL_VERSION_ATLEAST(2, 0, 3)
            case SDL_SYSWM_WINRT:     subsystem = "WinRT";                  break;
#endif
            case SDL_SYSWM_DIRECTFB:  subsystem = "DirectFB";               break;
            case SDL_SYSWM_COCOA:     subsystem = "Apple OS X";             break;
            case SDL_SYSWM_UIKIT:     subsystem = "UIKit";                  break;
#if SDL_VERSION_ATLEAST(2, 0, 2)
            case SDL_SYSWM_WAYLAND:   subsystem = "Wayland";                break;
            case SDL_SYSWM_MIR:       subsystem = "Mir";                    break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
            case SDL_SYSWM_ANDROID:   subsystem = "Android";                break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 5)
            case SDL_SYSWM_VIVANTE:   subsystem = "Vivante";                break;
#endif
        }

        console::cprintf("This program is running SDL version %u.%u.%u on %s\n", sys_windows_info.version.major, 
            sys_windows_info.version.minor, sys_windows_info.version.patch, subsystem);
    }
    else 
    {
        console::cprintf("Couldn't get window information: %s\n", SDL_GetError());
    }

    // Set context for SDL to use. Let SDL know that this window is the window that the OpenGL context should be tied to; everything that is drawn should be drawn to this window.
    if ((opengl_context = SDL_GL_CreateContext(window)) == nullptr)
    {
        printf("Failed to create OpenGL Context with SDL.\n");
        return false;
    }
    console::cprintf("OpenGL context created.\n");

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Enable us to access modern opengl extension features
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW failed to initialize.\n");
        return false;
    }
    console::cprintf("GLEW initialized.\n");

    gl_update_viewport_size();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // alpha blending func: a * (rgb) + (1 - a) * (rgb) = final color output
    glBlendEquation(GL_FUNC_ADD);
    sdl_vsync(0);                               // vsync
    SDL_SetRelativeMouseMode(SDL_TRUE);         // Lock mouse to window
    g_keystate = SDL_GetKeyboardState(nullptr); // Grab keystate array
    stbi_set_flip_vertically_on_load(true);     // stb_image setting
    kctta_setflags(KCTTA_CREATE_INDEX_BUFFER);  // kc_truetypeassembler setting

    // LOAD FONTS
    win64_load_font(&g_font_handle_c64, g_font_atlas_c64, "data/fonts/SourceCodePro.ttf", console::CON_TEXT_SIZE);

    console::initialize(&g_font_handle_c64, g_font_atlas_c64);
    profiler::initialize(&g_font_handle_c64, g_font_atlas_c64);
    debug::initialize();

    return true;
}

/** Process input and SDL events */
internal void game_process_events()
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
                b_is_game_running = false;
            } break;

            case SDL_WINDOWEVENT:
            {
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        gl_update_viewport_size();
                        calculate_perspectivematrix(g_camera, 90.f);
                        g_matrix_projection_ortho = projection_matrix_orthographic_2d(0.0f, (real32)g_buffer_width, (real32)g_buffer_height, 0.0f);
                    } break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        gl_update_viewport_size();
                        calculate_perspectivematrix(g_camera, 90.f);
                        g_matrix_projection_ortho = projection_matrix_orthographic_2d(0.0f, (real32)g_buffer_width, (real32)g_buffer_height, 0.0f);
                    } break;
                }
            } break;

            case SDL_MOUSEWHEEL:
            {
                if(console::is_shown() && g_curr_mouse_pos_y < console::CON_HEIGHT)
                {
                    if(event.wheel.y > 0)
                    {
                        console::scroll_up();
                    }
                    else if(event.wheel.y < 0)
                    {
                        console::scroll_down();
                    }
                }
            } break;

            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    console::toggle();
                    break;
                }

                if (console::is_shown())
                {
                    console::keydown(event.key);
                    break;
                }

                if (event.key.keysym.sym == SDLK_ESCAPE && console::is_hidden())
                {
                    b_is_game_running = false;
                    break;
                }

                if (event.key.keysym.sym == SDLK_F1)
                {
                    profiler::perf_profiler_level ? console::command("profiler 0") : console::command("profiler 1");
                    break;
                }

                if (event.key.keysym.sym == SDLK_F2)
                {
                    debug::debugger_level ? console::command("debug 0") : console::command("debug 1");
                    break;
                }

                if (event.key.keysym.sym == SDLK_z)
                {
                    SDL_SetRelativeMouseMode((SDL_bool) !b_relative_mouse);
                    console::cprintf("mouse grab = %s\n", !b_relative_mouse ? "true" : "false");
                    break;
                }
            } break;
        }
    }
}

/** Tick game logic. Delta time is in seconds. */
internal void game_update(real32 dt)
{
    console::update(dt);

    if(b_is_update_running)
    {
        update_camera(g_camera, dt);
    }
}

/** Process graphics and render them to the screen. */
internal void game_render()
{
    //glClearColor(0.39f, 0.582f, 0.926f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear opengl context's buffer

// NOT ALPHA BLENDED
    glDisable(GL_BLEND);
// DEPTH TESTED
    glEnable(GL_DEPTH_TEST);
    // TODO Probably should make own shader for wireframe draws so that wireframe fragments aren't affected by lighting or textures
    if(g_b_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    calculate_viewmatrix(g_camera);
    
    gl::use_shader(shader_common);

        gl::bind_view_matrix(shader_common, g_camera.matrix_view.ptr());
        gl::bind_projection_matrix(shader_common, g_camera.matrix_perspective.ptr());
        gl::bind_camera_position(shader_common, g_camera);

        temp_map_t* loaded_map = &loaded_maps[active_map_index];
        gl::bind_directional_light(shader_common, loaded_map->directionallight);
        gl::bind_point_lights(shader_common, loaded_map->pointlights.data(), (uint8)loaded_map->pointlights.size());
        gl::bind_spot_lights(shader_common, loaded_map->spotlights.data(), (uint8)loaded_map->spotlights.size());

        /** We could simply update the game object's position, rotation, scale fields,
            then construct the model matrix in game_render based on those fields.
        */
        mat4 matrix_model = identity_mat4();
        matrix_model = identity_mat4();
        matrix_model *= translation_matrix(loaded_map->mainobject.pos);
        matrix_model *= rotation_matrix(loaded_map->mainobject.orient);
        matrix_model *= scale_matrix(loaded_map->mainobject.scale);
        gl::bind_model_matrix(shader_common, matrix_model.ptr());

        gl::bind_material(shader_common, material_dull);
        render_mesh_group(loaded_map->mainobject.model);

    glUseProgram(0);

// ALPHA BLENDED
    glEnable(GL_BLEND);
    debug::render(shader_simple);

// NOT DEPTH TESTED
    glDisable(GL_DEPTH_TEST); 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    profiler::render(shader_ui, shader_text);
    console::render(shader_ui, shader_text);

    /* Swap our buffer to display the current contents of buffer on screen. 
    This is used with double-buffered OpenGL contexts, which are the default. */
    SDL_GL_SwapWindow(window);
}

void calc_average_normals(uint32* indices,
                          uint32 indices_count,
                          real32* vertices,
                          uint32 vertices_count,
                          uint32 vertex_size,
                          uint32 normal_offset)
{
    for(size_t i = 0; i < indices_count; i += 3)
    {
        uint32 in0 = indices[i] * vertex_size;
        uint32 in1 = indices[i+1] * vertex_size;
        uint32 in2 = indices[i+2] * vertex_size;
        vec3 v0 = make_vec3(vertices[in1] - vertices[in0],
                                 vertices[in1+1] - vertices[in0+1],
                                 vertices[in1+2] - vertices[in0+2]);
        vec3 v1 = make_vec3(vertices[in2] - vertices[in0],
                                 vertices[in2+1] - vertices[in0+1],
                                 vertices[in2+2] - vertices[in0+2]);
        vec3 normal = normalize(cross(v0, v1));

        in0 += normal_offset;
        in1 += normal_offset;
        in2 += normal_offset;
        vertices[in0] += normal.x;
        vertices[in0+1] += normal.y;
        vertices[in0+2] += normal.z;
        vertices[in1] += normal.x;
        vertices[in1+1] += normal.y;
        vertices[in1+2] += normal.z;
        vertices[in2] += normal.x;
        vertices[in2+1] += normal.y;
        vertices[in2+2] += normal.z;
    }

    for(size_t i = 0; i < vertices_count/vertex_size; ++i)
    {
        uint32 v_normal_offset = (int32)i * vertex_size + normal_offset;
        vec3 norm_vec = normalize(make_vec3(vertices[v_normal_offset],
                                            vertices[v_normal_offset+1],
                                            vertices[v_normal_offset+2]));
        vertices[v_normal_offset] = -norm_vec.x;
        vertices[v_normal_offset+1] = -norm_vec.y;
        vertices[v_normal_offset+2] = -norm_vec.z;
    }
}

int main(int argc, char* argv[]) // Our main entry point MUST be in this form when using SDL
{
    if (game_init() == false) return 1;

    uint32 indices[12] = { 
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices[32] = {
    //  x     y     z    u    v    normals
        -1.f, -1.f, -0.6f, 0.f, 0.f, 0.f, 0.f, 0.f,
        0.f, -1.f, 1.f, 0.5f, 0.f, 0.f, 0.f, 0.f,
        1.f, -1.f, -0.6f, 1.f, 0.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.5f, 1.f, 0.f, 0.f, 0.f
    };

    uint32 floor_indices[6] = { 
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floor_vertices[32] = {
        -10.f, 0.f, -10.f, 0.f, 0.f, 0.f, 1.f, 0.f,
        10.f, 0.f, -10.f, 10.f, 0.f, 0.f, 1.f, 0.f,
        -10.f, 0.f, 10.f, 0.f, 10.f, 0.f, 1.f, 0.f,
        10.f, 0.f, 10.f, 10.f, 10.f, 0.f, 1.f, 0.f
    };

    calc_average_normals(indices, 12, vertices, 32, 8, 5);

    gl::load_shader_program_from_file(shader_common, vertex_shader_path, frag_shader_path);
    gl::load_shader_program_from_file(shader_text, text_vs_path, text_fs_path);
    gl::load_shader_program_from_file(shader_ui, ui_vs_path, ui_fs_path);
    gl::load_shader_program_from_file(shader_simple, simple_vs_path, simple_fs_path);

    g_camera.position = { 0.f, 0.f, 0.f };
    g_camera.rotation = { 0.f, 270.f, 0.f };
    calculate_perspectivematrix(g_camera, 90.f);
    g_matrix_projection_ortho = projection_matrix_orthographic_2d(0.0f, (real32)g_buffer_width, (real32)g_buffer_height, 0.0f);

    // TEMPORARY setting up maps/scenes (TODO replace once we are loading maps from disk)
    loaded_maps[0].directionallight.orientation = direction_to_orientation(make_vec3(2.f, -1.f, -2.f));
    loaded_maps[0].directionallight.ambient_intensity = 0.3f;
    loaded_maps[0].directionallight.diffuse_intensity = 1.0f;
    loaded_maps[0].directionallight.colour = { 255.f/255.f, 231.f/255.f, 155.f/255.f };
    loaded_maps[0].temp_obj_path = "data/models/sponza/sponza.obj";
    loaded_maps[0].mainobject.scale = make_vec3(0.04f, 0.04f, 0.04f);
    loaded_maps[0].cam_start_pos = make_vec3(14.f, 6.f, -1.5f);
    loaded_maps[0].cam_start_rot = make_vec3(0.f, 180.f, 0.f);

    loaded_maps[1].directionallight.orientation = direction_to_orientation(make_vec3(2.f, -1.f, -2.f));
    loaded_maps[1].directionallight.ambient_intensity = 0.3f;
    loaded_maps[1].directionallight.diffuse_intensity = 1.0f;
    loaded_maps[1].directionallight.colour = { 255.f/255.f, 231.f/255.f, 155.f/255.f };
    loaded_maps[1].temp_obj_path = "data/models/doomguy/doommarine.obj";
    loaded_maps[1].mainobject.scale = make_vec3(0.04f, 0.04f, 0.04f);
    loaded_maps[1].cam_start_pos = make_vec3(0.f, 7.6f, 6.3f);
    loaded_maps[1].cam_start_rot = make_vec3(-21.f, -90.f, 0.f);

    loaded_maps[2].directionallight.orientation = direction_to_orientation(make_vec3(2.f, -1.f, -2.f));
    loaded_maps[2].directionallight.ambient_intensity = 0.3f;
    loaded_maps[2].directionallight.diffuse_intensity = 1.0f;
    loaded_maps[2].directionallight.colour = { 255.f/255.f, 231.f/255.f, 155.f/255.f };
    loaded_maps[2].temp_obj_path = "data/models/Alduin/Alduin.obj";
    loaded_maps[2].mainobject.scale = make_vec3(0.1f, 0.1f, 0.1f);
    loaded_maps[2].cam_start_pos = make_vec3(-63.f, 15.f, -14.5f);
    loaded_maps[2].cam_start_rot = make_vec3(9.69f, 13.6f, 0.f);

    loaded_maps[3].directionallight.orientation = direction_to_orientation(make_vec3(2.f, -1.f, -2.f));
    loaded_maps[3].directionallight.ambient_intensity = 0.3f;
    loaded_maps[3].directionallight.diffuse_intensity = 1.0f;
    loaded_maps[3].directionallight.colour = { 255.f/255.f, 231.f/255.f, 155.f/255.f };
    loaded_maps[3].temp_obj_path = "data/models/gallery/gallery.obj";
    loaded_maps[3].mainobject.orient = make_quaternion_deg(-89.f, make_vec3(1.f, 0.f, 0.f));
    loaded_maps[3].mainobject.scale = make_vec3(10.f, 10.f, 10.f);
    loaded_maps[3].cam_start_pos = make_vec3(-30.f, 31.f, -70.f);
    loaded_maps[3].cam_start_rot = make_vec3(-6.8f, -306.f, 0.f);

    game_switch_map(0);

    // point_lights[0].colour = { 0.0f, 1.0f, 0.0f };
    // point_lights[0].position = { -4.f, 0.0f, 0.0f };
    // point_lights[0].ambient_intensity = 0.f;
    // point_lights[0].diffuse_intensity = 1.f;
    // point_lights[1].colour = { 0.0f, 0.0f, 1.0f };
    // point_lights[1].position = { 4.f, 0.0f, 0.0f };
    // point_lights[1].ambient_intensity = 0.f;
    // point_lights[1].diffuse_intensity = 1.f;
    // debugger_set_pointlights(point_lights, array_count(point_lights));
    // spot_lights[0].position = { -4.f, 0.f, 0.f };
    // spot_lights[0].ambient_intensity = 0.f;
    // spot_lights[0].diffuse_intensity = 1.f;
    // spot_lights[0].set_cutoff_in_degrees(45.f);
    // spot_lights[0].orientation = direction_to_orientation(make_vec3(-1.f, -1.f, 0.f));
    // spot_lights[1].position = { -2.f, 0.f, 0.f };
    // spot_lights[1].ambient_intensity = 0.f;
    // spot_lights[1].diffuse_intensity = 1.f;
    // //spot_lights[1].set_cutoff_in_degrees(45.f);
    // spot_lights[1].orientation = direction_to_orientation(make_vec3(0.f, -1.f, 0.f));
    // debugger_set_spotlights(spot_lights, array_count(spot_lights));

    // Game Loop
    int64 perf_counter_frequency = timer::counter_frequency();
    int64 last_tick = timer::get_ticks(); // cpu cycles count of last tick
    while (b_is_game_running)
    {
        game_process_events();
        if (b_is_game_running == false) { break; }
        int64 this_tick = timer::get_ticks();
        int64 delta_tick = this_tick - last_tick;
        real32 deltatime_secs = (real32) delta_tick / (real32) perf_counter_frequency;
        last_tick = this_tick;
        profiler::perf_frametime_secs = deltatime_secs;
        game_update(deltatime_secs);
        game_render();
    }

    console::cprintf("Game shutting down...\n");

    // Cleanup
    gl::delete_shader(shader_common);
    gl::delete_shader(shader_text);
    gl::delete_shader(shader_ui);

    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(opengl_context);
    SDL_Quit();

    return 0;
}