cmake_minimum_required(VERSION 3.16)
project(VocAudioTest C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Try find raylib
find_package(raylib QUIET)

# If not found, fetch it
if(NOT raylib_FOUND)
    include(FetchContent)
    message(STATUS "raylib not found, fetching via FetchContent...")
    FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)
    set(raylib_FOUND TRUE)  # Set this manually after fetch
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES
        src/*.c
        test/*.c
)

add_executable(voc_test ${SRC_FILES})

# Link raylib - it should always be available now
target_link_libraries(voc_test PRIVATE raylib)

# On Linux, link math and pthreads maybe
if(UNIX)
    target_link_libraries(voc_test PRIVATE m pthread dl)
endif()

# Install (optional)
install(TARGETS voc_test RUNTIME DESTINATION bin)
