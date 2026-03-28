cmake_minimum_required(VERSION 3.20)
project(VoxelForge VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# --- Dependencies ---

# GLFW (windowing + input)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)

# GLM (math)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# GLAD2 (OpenGL 3.3 Core loader — requires Python 3)
FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    SOURCE_SUBDIR  cmake
)

FetchContent_MakeAvailable(glfw glm glad)

glad_add_library(glad_gl_core_33 REPRODUCIBLE LOADER API gl:core=3.3)

# --- VoxelForge executable ---

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

add_executable(VoxelForge ${SOURCES})

target_include_directories(VoxelForge PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(VoxelForge PRIVATE
    glfw
    glm
    glad_gl_core_33
)

# Copy assets next to the executable
add_custom_command(TARGET VoxelForge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:VoxelForge>/assets
)
