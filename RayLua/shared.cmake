# Shared definitions and sources
# Core files
set(C_SOURCES
        interfaces/engineapi.c
        core/mapcore.c
        core/artloader.c
        core/kplib.c
        Core/loaders.c
        Core/physics.c
        Core/monoclip.c
        Core/scenerender.c
        Core/shadowtest2.c
        Core/buildmath.c
        Core/monodebug.c
        interfaces/eventstore.c
)
set(C_HEADERS
        core/mapcore.h
        core/artloader.h
        core/loaders.h
        core/kplib.h
        core/physics.h
        Core/monoclip.h
        Core/scenerender.h
        Core/buildmath.h
        Core/shadowtest2.h
        interfaces/engineapi.h
        interfaces/shared_types.h
        interfaces/eventstore.h
        interfaces/ev_projection.h
        interfaces/event_types.h
        Core/monodebug.h
)
set(CORE_HEADERS
        dumbrender.hpp
        DumbCore.hpp
)
set(CORE_SOURCES
)

if (NOT DISABLE_LUA)
    list(APPEND CORE_HEADERS luabinder.hpp)
endif ()

# Shared compile definitions
function(add_shared_definitions target_name)
    target_compile_definitions(${target_name} PRIVATE
            USEHEIMAP=0
            NOSOUND=0
            OOS_CHECK=0
            _WIN32
            _WINDOWS
    )
endfunction()

# Shared C properties
function(set_c_properties)
    set_source_files_properties(${C_SOURCES} PROPERTIES
            LANGUAGE C
            C_STANDARD 99
            C_STANDARD_REQUIRED ON
    )
endfunction()

# Shared libraries
function(link_shared_libs target_name)
    target_link_libraries(${target_name}
            raylib
            imgui
            rlimgui
    )
    if (NOT DISABLE_LUA)
        target_link_libraries(${target_name} luajit_lib)
    endif()
endfunction()
