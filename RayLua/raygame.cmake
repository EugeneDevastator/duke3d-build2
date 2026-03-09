add_executable(RayGame
        main.cpp
        FileWatcher.h
        ${CORE_SOURCES}
        ${CORE_HEADERS}
        ${C_HEADERS}
        ${C_SOURCES}
        MonoTest.hpp
        DumbEdit.hpp
        Editor/uimodels.h
        Core/rendertypes.h

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
