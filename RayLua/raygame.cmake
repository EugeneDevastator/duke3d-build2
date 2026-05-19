add_executable(RayGame
        main.cpp
        ${CORE_SOURCES}
        ${C_SOURCES}
        Core/mapserial.c
)

# Mark header-only files so CMake doesn't compile them standalone
set_source_files_properties(
        ${CORE_HEADERS} ${C_HEADERS}
        MonoTest.hpp DumbEdit.hpp FileWatcher.h
        Editor/uimodels.h Core/rendertypes.h Core/mapserial.h
    PROPERTIES HEADER_FILE_ONLY TRUE
)

add_shared_definitions(RayGame)
set_c_properties()

target_include_directories(RayGame PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/core
)

if (NOT DISABLE_LUA)
    target_include_directories(RayGame PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src
    )
endif()

link_shared_libs(RayGame)

add_custom_command(TARGET RayGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/script.lua
        $<TARGET_FILE_DIR:RayGame>/script.lua)
