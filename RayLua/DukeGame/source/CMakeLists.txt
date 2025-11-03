cmake_minimum_required(VERSION 3.10)
project(DukeRuntime C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)



# Source files
set(SOURCES
        gamedef.c
        ENGINE.C
        actors.C
        player.C
        build.C
        KDMENG.C
        GAME.C
        keyboard.c
        global.c
        Cache1d.c
        PREMAP.C
        SOUNDS.C
        Config.c
        scriplib.c
        mmulti.c
        GAME_TEXT.c
        # Add more .c files here
)

# Header files (optional, for IDE support)
set(HEADERS
        build.h
        duke3d.h
        gamedefs.h
        config.h
        names.h
        engine.h
        sounds.h
        keyboard.h
        MUSIC.H
        funct.h
        control.h
        MMULTI.H
        pragmas.h
        MATHUTIL.H
        CACHE1D.H


        # Add more .h files here
)

# Include directories
include_directories(include)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
# Add compiler-specific flags
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -std=c99)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c11)
endif()
# Optional: Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
