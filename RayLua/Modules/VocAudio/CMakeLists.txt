cmake_minimum_required(VERSION 3.16)
project(VocAudioTest C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# try find raylib
find_package(raylib QUIET)

# If not found and we're on Windows, try to fetch it (optional)
if(NOT raylib_FOUND AND WIN32)
    include(FetchContent)
    message(STATUS "raylib not found, fetching via FetchContent...")
    FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)
endif()

if(NOT raylib_FOUND)
    message(STATUS "Warning: raylib not found via find_package(). Make sure raylib is installed and raylibConfig.cmake is findable.")
endif()

# include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES
        src/*.c
        test/*.c
)

add_executable(voc_test ${SRC_FILES})

# Link raylib if found
if(raylib_FOUND)
    target_link_libraries(voc_test PRIVATE raylib)
else()
    message(WARNING "raylib not found. You will need to set raylib_DIR or install raylib.")
endif()

# On Linux, link math and pthreads maybe
if (UNIX)
    target_link_libraries(voc_test PRIVATE m pthread dl)
endif()

# Install (optional)
install(TARGETS voc_test RUNTIME DESTINATION bin)
