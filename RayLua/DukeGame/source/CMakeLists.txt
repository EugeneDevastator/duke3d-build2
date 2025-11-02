cmake_minimum_required(VERSION 3.10)
project(DukeRuntime C)

set(CMAKE_C_STANDARD 99)

# Source files
set(SOURCES
        gamedef.c
        ENGINE.C;
        actors.C;
        players.C;
        build.C;
        #game.c
        KDMENG.C
        BUILD.C
        GAME_NOSYS.C;

        # Add more .c files here
)

# Header files (optional, for IDE support)
set(HEADERS
        build.h;
        duke3d.h;
        gamedefs.h;
        names.h;
        engine.h
        PREMAP.C
        SOUNDS.C
        keyboard.h
        MMULTI.H
        MUSIC.H

        # Add more .h files here
)

# Include directories
include_directories(include)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Optional: Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
