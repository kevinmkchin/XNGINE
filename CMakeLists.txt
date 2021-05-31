cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(opengldemo)

add_executable(${PROJECT_NAME} src/main_win64.cpp)
# add WIN32 after ${PROJECT_NAME} if compile for SUBSYSTEM:WINDOWS

add_compile_definitions(/DINTERNAL_BUILD=1 /DSLOW_BUILD=1)

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