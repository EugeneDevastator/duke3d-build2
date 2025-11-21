
# Collect Duke game source files
file(GLOB DUKE_C "DukeGame/source/*.c")
file(GLOB DUKE_H "DukeGame/source/*.h")

add_executable(DukeGame
        main.cpp
        FileWatcher.h
        ${CORE_SOURCES}
        ${CORE_HEADERS}
        ${C_HEADERS}
        ${C_SOURCES}
        ${DUKE_C}
        ${DUKE_H}
)

add_shared_definitions(DukeGame)
target_compile_definitions(DukeGame PRIVATE IS_DUKE_INCLUDED)

set_source_files_properties(${C_SOURCES} ${DUKE_C} PROPERTIES
        LANGUAGE C
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
)

target_include_directories(DukeGame PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/DukeGame
)

if (NOT DISABLE_LUA)
    target_include_directories(DukeGame PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src
    )
endif()

link_shared_libs(DukeGame)

add_custom_command(TARGET DukeGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/script.lua
        $<TARGET_FILE_DIR:DukeGame>/script.lua)
