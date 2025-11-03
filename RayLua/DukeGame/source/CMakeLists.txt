cmake_minimum_required(VERSION 3.10)
project(DukeRuntime C)

set(CMAKE_C_STANDARD 99)

# Source files
set(SOURCES
        gamedef.c
        ENGINE.C
        actors.C
        player.C
        build.C
        #game.c
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
        PLAYER.C
        MATHUTIL.H
        CACHE1D.H
        GAME_TEXT.c
        GAME_TEXT.c

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
