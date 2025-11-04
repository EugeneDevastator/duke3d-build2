cmake_minimum_required(VERSION 3.10)
project(Game C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Source files
set(SOURCES
        actors.c
        build.c
        cache1d.c
        config.c
        control.c
        engine.c
        game.c
        gamedefs.c
        game_text.c
        global.c
        kdmeng.c
        keyboard.c
        mmulti.c
        nomusic.c
        player.c
        premap.c
        scriplib.c
        sector.c
        sounds.c
)

# Header files (optional, for IDE support)
set(HEADERS

        build.h
        cache1d.h
        config.h
        control.h
        duke3d.h
        engine.h
        file_lib.h
        funct.h
        function.h
        fx_man.h
        gamedefs.h
        game_text.h
        global.h
        keyboard.h
        menues.h
        mmulti.h
        music.h
        names.h
        pragmas.h
        scriplib.h
        sector.h
        soundefs.h
        sounds.h
        task_man.h
        types.h
        _functio.h

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
