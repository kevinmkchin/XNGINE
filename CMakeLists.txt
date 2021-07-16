cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(xngine)

add_executable(${PROJECT_NAME}
        src/main_win64.cpp
        src/renderer/light.cpp
        src/renderer/mesh.cpp
        src/renderer/camera.cpp
        src/renderer/texture.cpp
        src/renderer/mesh_group.cpp
        src/renderer/material.cpp
        src/core/timer_win64.cpp
        src/core/file_system_win64.cpp
        src/debugging/profiling/profiler.cpp
        src/debugging/console.cpp
        src/debugging/debug_drawer.cpp
        src/debugging/commands.cpp
        src/core/input.cpp
        src/core/display_SDL.cpp
        src/renderer/render_manager.cpp
        src/runtime/game_state.cpp
        src/renderer/shader.cpp)
# add WIN32 after ${PROJECT_NAME} if compile for SUBSYSTEM:WINDOWS

# Compile time preprocessor definitions
add_compile_definitions(
        INTERNAL_BUILD=1
        SLOW_BUILD=1
)

# Additional Include Directories
include_directories(${PROJECT_NAME}
    PUBLIC lib/GLEW/include
    PUBLIC lib/SDL/include
    PUBLIC lib/ASSIMP/include
)

# Additional Library/Dependency Directories
target_link_directories(${PROJECT_NAME} 
    PRIVATE lib/GLEW/lib/Release/x64
    PRIVATE lib/SDL/lib/x64
    PRIVATE lib/ASSIMP/lib
)

# Additional Dependencies
target_link_libraries(${PROJECT_NAME}
    Shell32.lib
    opengl32.lib
    glew32.lib
    SDL2.lib
    SDL2main.lib
    assimp-vc142-mt.lib
)