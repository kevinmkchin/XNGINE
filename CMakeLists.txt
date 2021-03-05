cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(opengldemo)

add_executable(${PROJECT_NAME} src/main.cpp)

# Additional Include Directories
include_directories(${PROJECT_NAME}
    PUBLIC libs/GLEW/include
    PUBLIC libs/SDL/include
    PUBLIC libs/GLM
)

# Additional Library/Dependency Directories
target_link_directories(${PROJECT_NAME} 
    PRIVATE libs/GLEW/lib/Release/x64
    PRIVATE libs/SDL/lib/x64
)

# Additional Dependencies
target_link_libraries(${PROJECT_NAME}
    Shell32.lib
    opengl32.lib
    glew32.lib
    SDL2.lib
    SDL2main.lib
)