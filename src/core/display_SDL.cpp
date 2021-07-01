#include <SDL.h>
#include <SDL_syswm.h>
#include <cstdio>
#include "display.h"
#include "../debugging/console.h"
#include "../runtime/game_state.h"
#include "../renderer/render_manager.h"

SINGLETON_INIT(display)

internal SDL_Window* window = nullptr;
internal SDL_GLContext opengl_context = nullptr;

void display::initialize()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL failed to initialize.\n");
        // todo return false;
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
        // todo return false;
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

        console_printf("This program is running SDL version %u.%u.%u on %s\n", sys_windows_info.version.major,
                       sys_windows_info.version.minor, sys_windows_info.version.patch, subsystem);
    }
    else
    {
        console_printf("Couldn't get window information: %s\n", SDL_GetError());
    }

    // Set context for SDL to use. Let SDL know that this window is the window that the OpenGL context should be tied to;
    // everything that is drawn should be drawn to this window.
    if ((opengl_context = SDL_GL_CreateContext(window)) == nullptr)
    {
        printf("Failed to create OpenGL Context with SDL.\n");
        // todo return false;
    }
    console_printf("OpenGL context created.\n");

    vsync(0);

    display_size_changed();
}

void display::swap_buffers()
{
    SDL_GL_SwapWindow(window);
}

void display::clean_up()
{
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(opengl_context);
    SDL_Quit();
}

void display::vsync(int vsync)
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
        default:{
            console_print("Invalid vsync option; 0 = immediate, 1 = vsync, 2 = adaptive vsync");
        }break;
    }

}

void display::display_size_changed()
{
    /** Get the size of window's underlying drawable in pixels (for use with glViewport).
    Remark: This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI drawable, i.e. the window was created
    with SDL_WINDOW_ALLOW_HIGHDPI on a platform with high-DPI support (Apple calls this "Retina"), and not disabled by the
    SDL_HINT_VIDEO_HIGHDPI_DISABLED hint. */
    i32 buffer_width;
    i32 buffer_height;
    SDL_GL_GetDrawableSize(window, &buffer_width, &buffer_height);
    render_manager::get_instance()->update_buffer_size(buffer_width, buffer_height);

    render_manager::get_instance()->matrix_projection_ortho =
            projection_matrix_orthographic_2d(0.0f, (float)buffer_width, (float)buffer_height, 0.0f);
    render_manager::get_instance()->gs->m_camera.calculate_perspective_matrix();
}
