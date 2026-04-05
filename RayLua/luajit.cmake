# LuaJIT linkage. Native Linux builds should use system packages; the checked-in
# Windows path still supports the original MSVC workflow.
if (NOT DISABLE_LUA)
    if (WIN32)
        set(LUAJIT_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src/lua51.lib)

        if (NOT EXISTS ${LUAJIT_LIB_PATH})
            message(FATAL_ERROR "LuaJIT for Windows was not found at ${LUAJIT_LIB_PATH}. Build or vendor LuaJIT before enabling Lua.")
        endif ()

        add_library(luajit_lib STATIC IMPORTED)
        set_target_properties(luajit_lib PROPERTIES
            IMPORTED_LOCATION ${LUAJIT_LIB_PATH}
        )
        target_include_directories(luajit_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src)
    else ()
        find_path(LUAJIT_INCLUDE_DIR luajit.h PATH_SUFFIXES luajit-2.1 luajit-2.0)
        find_library(LUAJIT_LIBRARY NAMES luajit-5.1 luajit)

        if (NOT LUAJIT_INCLUDE_DIR OR NOT LUAJIT_LIBRARY)
            message(FATAL_ERROR "Lua support requested, but system LuaJIT was not found. Install luajit and luajit headers, or configure with -DDISABLE_LUA=ON.")
        endif ()

        add_library(luajit_lib UNKNOWN IMPORTED)
        set_target_properties(luajit_lib PROPERTIES
            IMPORTED_LOCATION ${LUAJIT_LIBRARY}
        )
        target_include_directories(luajit_lib INTERFACE ${LUAJIT_INCLUDE_DIR})
    endif ()
endif ()
