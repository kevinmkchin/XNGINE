/** OpenGL 3D Renderer

TODO:
    - Review Texture.cpp functions and clean up. Refactor out stbi_load. 
    - Review Mesh.cpp functions and clean up
    - Review Camera.cpp functions and clean up
    - Review Shader.cpp functions and clean up
    - Review main.cpp functions and clean up

Backlog:
    - Quake-style console with extensible commands
        - use opengl textured quads for both the background and the text
        - shader hotloading/compiling during runtime
    - WATCH handmadehero day 14 first: memory.cpp Memory management system 
    (allocate and deallocate functions that use a memory arena or something instead of doing malloc all the time)
    - WATCH handmadehero day 15
    - Phong Lighting
    - Write own math library and remove GLM
    - Entity - pos, rot, scale, mesh, few boolean flags, collider, tags

    THIS PROJECT IS A SINGLE TRANSLATION UNIT BUILD / UNITY BUILD
*/
#include <stdio.h>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>

#include <gl/glew.h>
#include <SDL.h>
#include <Windows.h>
#include <SDL_syswm.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"           // Sean Barrett's stb_truetype library
#include "kc_truetypeassembler.h"   // KC TrueType Assembler

#include "gamedefine.h" // defines and typedefs
#include "core.h"
#include "data.h"
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

GLOBAL_VAR Camera camera;

GLOBAL_VAR bool is_running = true;
GLOBAL_VAR SDL_Window* window = nullptr;
GLOBAL_VAR SDL_GLContext opengl_context = nullptr;
GLOBAL_VAR HWND whandle = nullptr;  // Win32 API Window Handle
GLOBAL_VAR glm::mat4 matrix_projection;
GLOBAL_VAR glm::mat4 matrix_projection_ortho;
// -------------------------
#include "data.cpp"
#include "camera.cpp"
#include "mesh.cpp"
#include "shader.cpp"
#include "texture.cpp"

Mesh meshes[3];
ShaderProgram shaders[2];
Texture tex_brick;
Texture tex_dirt;

static const char* vertex_shader_path = "shaders/default.vert";
static const char* frag_shader_path = "shaders/default.frag";
static const char* ui_vs_path = "shaders/text_ui.vert";
static const char* ui_fs_path = "shaders/text_ui.frag";
TTAFont pogfont;
Texture tex_font_atlas;


INTERNAL void load_font(TTAFont& font_handle, const char* font_file_path, int font_height_in_pixels)
{
    BinaryFileHandle fontfile;
    file_read_file_binary(fontfile, font_file_path);
    if(fontfile.memory)
    {
        kctta_init_font(&font_handle, (uint8*) fontfile.memory, font_height_in_pixels);
    }
    file_free_file_binary(fontfile);
}

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


INTERNAL int8 game_run();
INTERNAL bool game_init();
INTERNAL void game_loop();
INTERNAL void game_process_events();
INTERNAL void game_update(real32 dt);
INTERNAL void game_render();
INTERNAL void game_clean_up();

/** Start the game procedure
 */
INTERNAL int8 game_run()
{
    if (game_init() == false) return 1;

    create_triangles();


    load_font(pogfont, "c:/windows/fonts/times.ttf", 30);
    gl_load_bitmap(tex_font_atlas, pogfont.font_atlas.pixels, pogfont.font_atlas.width, pogfont.font_atlas.height, GL_RED, GL_RED);
    free(pogfont.font_atlas.pixels);
    kctta_use_index_buffer(1);
    kctta_move_cursor(400, 300);
    kctta_append_line("Amogus", &pogfont, 100);
    TTAVertexBuffer textbuffer = kctta_grab_buffer();
    meshes[2] = gl_create_mesh_array(textbuffer.vertex_buffer, textbuffer.index_buffer, 
        textbuffer.vertices_array_count, textbuffer.indices_array_count, 2, 2);
    kctta_clear_buffer();


    camera.position = glm::vec3(0.f, 0.f, 0.f);
    camera.rotation = glm::vec3(0.f, 270.f, 0.f);

    ShaderProgram shader;
    init_shader_program(shader, vertex_shader_path, frag_shader_path);
    shaders[0] = shader;
    init_shader_program(shader, ui_vs_path, ui_fs_path);
    shaders[1] = shader;

    tex_brick.file_path = "data/textures/brick.png";
    gl_load_texture(tex_brick);
    tex_dirt.file_path = "data/textures/mqdefault.jpg";
    gl_load_texture(tex_dirt);

    /** Going to create the projection matrix here because we only need to create projection matrix once (as long as fov or aspect ratio doesn't change)
        The model matrix, right now, is in Game::render because we want to be able to update the object's transform on tick. However, ideally, the 
        model matrix creation and transformation should be done in Game::update because that's where we should be updating the object's transformation.
        That matrix can be stored inside the game object class alongside the VAO. Or we could simply update the game object's position, rotation, scale
        fields, then construct the model matrix in Game::render based on those fields. Yeah that's probably better.
    */
    real32 aspect_ratio = (real32)g_buffer_width / (real32)g_buffer_height;
    matrix_projection = glm::perspective(45.f, aspect_ratio, 0.1f, 1000.f);
    matrix_projection_ortho = glm::ortho(0.0f, (real32)g_buffer_width,(real32)g_buffer_height,0.0f, -100.f, 10.f);

    game_loop();
    game_clean_up();

    return 0;
}

/** Create window, set up OpenGL context, initialize SDL and GLEW 
*/
INTERNAL bool game_init()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL failed to initialize.");
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
        printf("SDL window failed to create.");
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

        SDL_Log("This program is running SDL version %u.%u.%u on %s", sys_windows_info.version.major, 
            sys_windows_info.version.minor, sys_windows_info.version.patch, subsystem);
    }
    else 
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't get window information: %s", SDL_GetError());
    }
    // Grab HWND from window info
    whandle = sys_windows_info.info.win.window;

    // Set context for SDL to use. Let SDL know that this window is the window that the OpenGL context should be tied to; everything that is drawn should be drawn to this window.
    if ((opengl_context = SDL_GL_CreateContext(window)) == nullptr)
    {
        printf("Failed to create OpenGL Context with SDL.");
        return false;
    }

    /** This makes our Buffer Swap (SDL_GL_SwapWindow) synchronized with the monitor's vertical refresh - basically vsync; 
        0 = immediate, 1 = vsync, -1 = adaptive vsync. Remark: If application requests adaptive vsync and the system does 
        not support it, this function will fail and return -1. In such a case, you should probably retry the call with 1 
        for the interval. */
    if (SDL_GL_SetSwapInterval(0) == -1)
    {
        SDL_GL_SetSwapInterval(1);
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Enable us to access modern opengl extension features
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW failed to initialize.");
        game_clean_up();
        return false;
    }

    /** Get the size of window's underlying drawable in pixels (for use with glViewport).
        Remark: This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI drawable, i.e. the window was created 
        with SDL_WINDOW_ALLOW_HIGHDPI on a platform with high-DPI support (Apple calls this "Retina"), and not disabled by the 
        SDL_HINT_VIDEO_HIGHDPI_DISABLED hint. */
    SDL_GL_GetDrawableSize(window, (int*)&g_buffer_width, (int*)&g_buffer_height);

    // Setup Viewport
    glViewport(0, 0, g_buffer_width, g_buffer_height);

    // Enable depth test to see which triangles should be drawn over other triangles
    glEnable(GL_DEPTH_TEST); 
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // a * (rgb) + (1 - a) * (rgb) = final color output
    glBlendEquation(GL_FUNC_ADD);
    
    // Lock mouse to window
    SDL_SetRelativeMouseMode(SDL_TRUE);
    // Grab keystate array
    g_keystate = SDL_GetKeyboardState(nullptr);

    return true;
}

/** Clear memory and shut down
 */
INTERNAL void game_clean_up()
{
    gl_delete_texture(tex_brick);
    gl_delete_texture(tex_dirt);
    // clear shaders and delete shaders
    for (int i = 0; i < array_count(meshes); ++i)
    {
        gl_delete_mesh(meshes[i]);
    }
    for (int i = 0; i < array_count(shaders); ++i)
    {
        gl_clear_shader(shaders[i]);
    }

    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(opengl_context);
    SDL_Quit();
}

/** Game loop with fixed timestep - Input, Logic, Render
 */
INTERNAL void game_loop()
{
    real32 last_tick = (real32)SDL_GetTicks();  // time (in milliseconds since SDL init) of the last tick
    while (is_running)
    {
        game_process_events();
        if (is_running == false) { break; }
        real32 this_tick = (real32)SDL_GetTicks();
        real32 delta_time_ms = this_tick - last_tick;
        game_update(delta_time_ms / 1000.f);
        last_tick = this_tick; // TODO is this right?
        game_render();
    }
}

/** Process input and SDL events
*/
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
                // TODO register keydown for text input into dropdown console

                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    is_running = false;
                    break;
                } 

                if (event.key.keysym.sym == SDLK_z)
                {
                    SDL_SetRelativeMouseMode((SDL_bool) !b_relative_mouse);
                    printf("mouse grab = %s\n", !b_relative_mouse ? "true" : "false");
                } 
            } break;
        }
    }
}

/** Tick game logic. Delta time is in seconds.
*/
INTERNAL void game_update(real32 dt)
{
    update_camera(camera, dt);
}

/** Process graphics and render them to the screen.
*/
INTERNAL void game_render()
{
    // Clear opengl context's buffer
    //glClearColor(0.39f, 0.582f, 0.926f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO Probably should make own shader for wireframe draws so that wireframe fragments aren't affected by lighting or textures
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // GL_LINE for wireframe, GL_FILL to fill interior of polygon

    use_shader(shaders[0]);

        glUniformMatrix4fv(shaders[0].id_uniform_view, 1, GL_FALSE, glm::value_ptr(calculate_viewmatrix(camera)));
        glUniformMatrix4fv(shaders[0].id_uniform_projection, 1, GL_FALSE, glm::value_ptr(matrix_projection));

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

    // test
    // kctta_move_cursor(100,100);
    // kctta_append_line("amogus", &pogfont, 100);
    // TTAVertexBuffer textbuffer = kctta_grab_buffer();
    // gl_rebind_buffers(meshes[2], 
    //     textbuffer.vertex_buffer, 
    //     textbuffer.index_buffer, 
    //     textbuffer.vertices_array_count, 
    //     textbuffer.indices_array_count,
    //     GL_DYNAMIC_DRAW);
    // kctta_clear_buffer();

    use_shader(shaders[1]);
        glUniformMatrix4fv(shaders[1].id_uniform_projection, 1, GL_FALSE, glm::value_ptr(matrix_projection_ortho));

        matrix_model = glm::mat4(1.f);
        matrix_model = glm::translate(matrix_model, glm::vec3(0.f, 0.f, 100.f));
        //matrix_model = glm::scale(matrix_model, glm::vec3(1.f, 3.f, 1.f));
        glUniformMatrix4fv(shaders[1].id_uniform_model, 1, GL_FALSE, glm::value_ptr(matrix_model));
        glUniform3f(glGetUniformLocation(shaders[1].id_shader_program, "text_colour"), 0.f, 1.f, 0.f);
        gl_use_texture(tex_font_atlas);
        gl_render_mesh(meshes[2]);
    glUseProgram(0);
    //

    /* Swap our buffer to display the current contents of buffer on screen. 
    This is used with double-buffered OpenGL contexts, which are the default. */
    SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[]) // Our main entry point MUST be in this form when using SDL
{
    return game_run();
}