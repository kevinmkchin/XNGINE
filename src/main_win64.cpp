/** OpenGL 3D Renderer

TODO:

Backlog:
    - WATCH handmadehero day 14 first: memory.cpp Memory management system 
    (allocate and deallocate functions that use a memory arena or something instead of doing malloc all the time)
    - WATCH handmadehero day 15
    - Phong Lighting
    - Write own math library and remove GLM
    - Entity - pos, rot, scale, mesh, few boolean flags, collider, tags
    - Resize event 
        https://stackoverflow.com/questions/20735637/resize-sdl2-window#:~:text=To%20resize%20a%20window%20in,0%2C%20windowWidth%2C%20windowHeight)%20.
        SDL_WINDOWEVENT_RESIZED https://wiki.libsdl.org/SDL_WindowEvent
    - Frame lock
    - Make shader not require certain uniform matrices like model view and projection
    - Texture GL_NEAREST option
    - Texture do something like source engine
        - Build simple polygons and shapes, and the textures get wrapped
          automatically(1 unit in vertices is 1 unit in texture uv)
    - Console:
        - remember previously entered commands
        - shader hotloading/compiling during runtime - pause all update / render while shaders are being recompiled
        - mouse picking entities

    THIS PROJECT IS A SINGLE TRANSLATION UNIT BUILD / UNITY BUILD
*/
#define DEBUG
#ifndef DEBUG
#define RELEASE
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <iostream>
#include <fstream>

#include <gl/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <profileapi.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "kc_truetypeassembler.h"

#include "gamedefine.h" // defines and typedefs
#include "core.h"
#include "data.h"
#include "console.h"
// --- global variables  --- note: static variables are initialized to their default values
GLOBAL_VAR uint32 g_buffer_width;
GLOBAL_VAR uint32 g_buffer_height;

GLOBAL_VAR const uint8* g_keystate = nullptr;       // Stores keyboard state this frame. Access via g_keystate[SDL_Scancode].

GLOBAL_VAR int32 g_last_mouse_pos_x = INDEX_NONE;   // Stores mouse state this frame. mouse_pos is not updated when using SDL RelativeMouseMode.
GLOBAL_VAR int32 g_last_mouse_pos_y = INDEX_NONE;
GLOBAL_VAR int32 g_curr_mouse_pos_x = INDEX_NONE;
GLOBAL_VAR int32 g_curr_mouse_pos_y = INDEX_NONE;
GLOBAL_VAR int32 g_mouse_delta_x;
GLOBAL_VAR int32 g_mouse_delta_y;

GLOBAL_VAR Camera g_camera;

GLOBAL_VAR bool is_running = true;
GLOBAL_VAR bool b_is_update_running = true;
GLOBAL_VAR SDL_Window* window = nullptr;
GLOBAL_VAR SDL_GLContext opengl_context = nullptr;
GLOBAL_VAR glm::mat4 g_matrix_projection_ortho;
GLOBAL_VAR bool g_b_wireframe = false;
// -------------------------
#include "data.cpp"
#include "camera.cpp"
#include "mesh.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "profiler.cpp"
#include "commands.cpp"
#include "console.cpp"

Mesh meshes[3];
ShaderProgram shaders[3];
Texture tex_brick;
Texture tex_dirt;

// - Fonts -
TTAFont g_font_handle_c64;
Texture g_font_atlas_c64;
// ---------

static const char* vertex_shader_path = "shaders/default.vert";
static const char* frag_shader_path = "shaders/default.frag";
static const char* ui_vs_path = "shaders/ui.vert";
static const char* ui_fs_path = "shaders/ui.frag";
static const char* text_vs_path = "shaders/text_ui.vert";
static const char* text_fs_path = "shaders/text_ui.frag";

INTERNAL void create_triangles()
{
    uint32 indices[12] = { 
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices[20] = {
    //  x     y     z    u    v
        -1.f, -1.f, 0.f, 0.f, 0.f,
        0.f, -1.f, 1.f, 0.5f, 0.f,
        1.f, -1.f, 0.f, 1.f, 0.f,
        0.f, 1.f, 0.f, 0.5f, 1.f
    };

    meshes[0] = gl_create_mesh_array(vertices, indices, 20, 12);
    meshes[1] = gl_create_mesh_array(vertices, indices, 20, 12);
}

INTERNAL inline int64 platform_get_ticks()
{
    // win64 version of get ticks
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
    {
        return -1;
    }
    return ticks.QuadPart;
}

INTERNAL int8 game_run();
INTERNAL bool game_init();
INTERNAL void game_loop();
INTERNAL void game_process_events();
INTERNAL void game_update(real32 dt);
INTERNAL void game_render();
INTERNAL void game_clean_up();

/** Start the game procedure */
INTERNAL int8 game_run()
{
    if (game_init() == false) return 1;

    create_triangles();

    ShaderProgram shader;
    gl_load_shader_program_from_file(shader, vertex_shader_path, frag_shader_path);
    shaders[0] = shader;
    gl_load_shader_program_from_file(shader, text_vs_path, text_fs_path);
    shaders[1] = shader;
    gl_load_shader_program_from_file(shader, ui_vs_path, ui_fs_path);
    shaders[2] = shader;

    gl_load_texture_from_file(tex_brick, "data/textures/brick.png");
    gl_load_texture_from_file(tex_dirt, "data/textures/mqdefault.jpg");

    /** Going to create the projection matrix here because we only need to create projection matrix once (as long as fov or aspect ratio doesn't change)
        The model matrix, right now, is in Game::render because we want to be able to update the object's transform on tick. However, ideally, the 
        model matrix creation and transformation should be done in Game::update because that's where we should be updating the object's transformation.
        That matrix can be stored inside the game object class alongside the VAO. Or we could simply update the game object's position, rotation, scale
        fields, then construct the model matrix in Game::render based on those fields. Yeah that's probably better.
    */
    real32 aspect_ratio = (real32)g_buffer_width / (real32)g_buffer_height;
    g_camera.matrix_perspective = glm::perspective(45.f, aspect_ratio, 0.1f, 1000.f);
    g_camera.position = glm::vec3(0.f, 0.f, 0.f);
    g_camera.rotation = glm::vec3(0.f, 270.f, 0.f);
    g_matrix_projection_ortho = glm::ortho(0.0f, (real32)g_buffer_width,(real32)g_buffer_height,0.0f, -0.1f, 0.f);

    game_loop();
    game_clean_up();

    return 0;
}

/** Create window, set up OpenGL context, initialize SDL and GLEW */
INTERNAL bool game_init()
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

    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

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

        con_printf("This program is running SDL version %u.%u.%u on %s\n", sys_windows_info.version.major, 
            sys_windows_info.version.minor, sys_windows_info.version.patch, subsystem);
    }
    else 
    {
        con_printf("Couldn't get window information: %s\n", SDL_GetError());
    }

    // Set context for SDL to use. Let SDL know that this window is the window that the OpenGL context should be tied to; everything that is drawn should be drawn to this window.
    if ((opengl_context = SDL_GL_CreateContext(window)) == nullptr)
    {
        printf("Failed to create OpenGL Context with SDL.\n");
        return false;
    }
    con_printf("OpenGL context created.\n");

    /** This makes our Buffer Swap (SDL_GL_SwapWindow) synchronized with the monitor's vertical refresh - basically vsync; 
        0 = immediate, 1 = vsync, -1 = adaptive vsync. Remark: If application requests adaptive vsync and the system does 
        not support it, this function will fail and return -1. In such a case, you should probably retry the call with 1 
        for the interval. */
    if (SDL_GL_SetSwapInterval(-1) == -1)
    {
        SDL_GL_SetSwapInterval(1);
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Enable us to access modern opengl extension features
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW failed to initialize.\n");
        game_clean_up();
        return false;
    }
    con_printf("GLEW initialized.\n");

    /** Get the size of window's underlying drawable in pixels (for use with glViewport).
        Remark: This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI drawable, i.e. the window was created 
        with SDL_WINDOW_ALLOW_HIGHDPI on a platform with high-DPI support (Apple calls this "Retina"), and not disabled by the 
        SDL_HINT_VIDEO_HIGHDPI_DISABLED hint. */
    SDL_GL_GetDrawableSize(window, (int*)&g_buffer_width, (int*)&g_buffer_height);

    // Setup Viewport
    glViewport(0, 0, g_buffer_width, g_buffer_height);
    con_printf("Viewport initialized - x: %d y: %d\n", g_buffer_width, g_buffer_height);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // a * (rgb) + (1 - a) * (rgb) = final color output
    glBlendEquation(GL_FUNC_ADD);
    
    // Lock mouse to window
    SDL_SetRelativeMouseMode(SDL_TRUE);
    // Grab keystate array
    g_keystate = SDL_GetKeyboardState(nullptr);

    // stb_image setting
    stbi_set_flip_vertically_on_load(true);
    // kc_truetypeassembler setting
    kctta_use_index_buffer(1);

    // LOAD FONTS
    BinaryFileHandle fontfile;
    FILE_read_file_binary(fontfile, "data/fonts/c64.ttf");
        if(fontfile.memory)
        {
            kctta_init_font(&g_font_handle_c64, (uint8*) fontfile.memory, CON_TEXT_SIZE);
        }
    FILE_free_file_binary(fontfile);
    gl_load_texture_from_bitmap(g_font_atlas_c64,
                                g_font_handle_c64.font_atlas.pixels,
                                g_font_handle_c64.font_atlas.width,
                                g_font_handle_c64.font_atlas.height,
                                GL_RED, GL_RED);
    free(g_font_handle_c64.font_atlas.pixels);

    con_initialize(&g_font_handle_c64, g_font_atlas_c64);
    profiler_initialize(&g_font_handle_c64, g_font_atlas_c64);

    return true;
}

/** Clear memory and shut down */
INTERNAL void game_clean_up()
{
    con_printf("Game shutting down...\n");

    gl_delete_texture(tex_brick);
    gl_delete_texture(tex_dirt);
    // clear shaders and delete shaders
    for (int i = 0; i < array_count(meshes); ++i)
    {
        gl_delete_mesh(meshes[i]);
    }
    for (int i = 0; i < array_count(shaders); ++i)
    {
        gl_delete_shader(shaders[i]);
    }

    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(opengl_context);
    SDL_Quit();
}

/** Game loop with fixed timestep - Input, Logic, Render */
INTERNAL void game_loop()
{
    LARGE_INTEGER perf_counter_frequency_result;
    QueryPerformanceFrequency(&perf_counter_frequency_result);
    int64 perf_counter_frequency = perf_counter_frequency_result.QuadPart;

    // Loop
    int64 last_tick = platform_get_ticks(); // cpu cycles count of last tick
    while (is_running)
    {
        game_process_events();
        if (is_running == false) { break; }
        int64 this_tick = platform_get_ticks();
        int64 delta_tick = this_tick - last_tick;
        real32 deltatime_secs = (real32) delta_tick / (real32) perf_counter_frequency;
        last_tick = this_tick;
        perf_gameloop_elapsed_secs = deltatime_secs;
        game_update(deltatime_secs);
        game_render();
    }
}

/** Process input and SDL events */
INTERNAL void game_process_events()
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
                is_running = false;
            } break;

            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_F11)
                {
                    break;
                }

                if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    con_toggle();
                    break;
                }

                if (con_is_shown())
                {
                    con_keydown(event.key);
                    break;
                }

                if (event.key.keysym.sym == SDLK_ESCAPE && con_is_hidden())
                {
                    is_running = false;
                    break;
                } 

                if (event.key.keysym.sym == SDLK_z)
                {
                    SDL_SetRelativeMouseMode((SDL_bool) !b_relative_mouse);
                    con_printf("mouse grab = %s\n", !b_relative_mouse ? "true" : "false");
                } 
            } break;
        }
    }
}

/** Tick game logic. Delta time is in seconds. */
INTERNAL void game_update(real32 dt)
{
    con_update(dt);

    if(b_is_update_running)
    {
        update_camera(g_camera, dt);
    }
}

/** Process graphics and render them to the screen. */
INTERNAL void game_render()
{
    //glClearColor(0.39f, 0.582f, 0.926f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear opengl context's buffer

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
    
    gl_use_shader(shaders[0]);
        glUniformMatrix4fv(shaders[0].id_uniform_view, 1, GL_FALSE, glm::value_ptr(calculate_viewmatrix(g_camera)));
        glUniformMatrix4fv(shaders[0].id_uniform_projection, 1, GL_FALSE, glm::value_ptr(g_camera.matrix_perspective));

        glm::mat4 matrix_model = glm::mat4(1.f);
        matrix_model = glm::translate(matrix_model, glm::vec3(0.f, 0.5f, -1.3f));
        matrix_model = glm::scale(matrix_model, glm::vec3(0.3f, 0.3f, 0.3f)); // scale in each axis by the respective values of the vector
        glUniformMatrix4fv(shaders[0].id_uniform_model, 1, GL_FALSE, glm::value_ptr(matrix_model));
        gl_use_texture(tex_brick);
        gl_render_mesh(meshes[0]);

        matrix_model = glm::mat4(1.f);
        matrix_model = glm::translate(matrix_model, glm::vec3(0.f, -0.5f, -1.3f));
        matrix_model = glm::scale(matrix_model, glm::vec3(0.3f, 0.3f, 0.3f));
        glUniformMatrix4fv(shaders[0].id_uniform_model, 1, GL_FALSE, glm::value_ptr(matrix_model));
        gl_use_texture(tex_dirt);
        gl_render_mesh(meshes[1]);
    glUseProgram(0);

// NOT DEPTH TESTED
    glDisable(GL_DEPTH_TEST); 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    profiler_render(shaders[2], shaders[1]);
    con_render(shaders[2], shaders[1]);

    /* Swap our buffer to display the current contents of buffer on screen. 
    This is used with double-buffered OpenGL contexts, which are the default. */
    SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[]) // Our main entry point MUST be in this form when using SDL
{
    return game_run();
}