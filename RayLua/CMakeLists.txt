cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21)

project(raylua C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(DISABLE_LUA "Disable Lua support" ON)
option(BUILD_DUKEGAME "Build the DukeGame target" OFF)

include(FetchContent)
find_package(OpenGL REQUIRED)

set(RAYLUA_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External)

# Prefer the system raylib package on Arch. Fall back to FetchContent if needed.
find_package(raylib QUIET)
if (NOT raylib_FOUND)
    message(STATUS "raylib not found via find_package(), fetching raylib 5.5")
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)
endif ()

if (EXISTS ${RAYLUA_EXTERNAL_DIR}/imgui/imgui.cpp)
    set(imgui_SOURCE_DIR ${RAYLUA_EXTERNAL_DIR}/imgui)
else ()
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.1
    )
    FetchContent_MakeAvailable(imgui)
endif ()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

if (EXISTS ${RAYLUA_EXTERNAL_DIR}/rlImGui/rlImGui.cpp)
    set(rlimgui_SOURCE_DIR ${RAYLUA_EXTERNAL_DIR}/rlImGui)
else ()
    FetchContent_Declare(
        rlimgui
        GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
        GIT_TAG 4d8a618
    )
    FetchContent_MakeAvailable(rlimgui)
endif ()

add_library(rlimgui STATIC ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
target_link_libraries(rlimgui PUBLIC raylib imgui)
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})

include(luajit.cmake)
include(shared.cmake)
include(raygame.cmake)

if (BUILD_DUKEGAME)
    include(duke_target.cmake)
endif ()
