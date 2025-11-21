# LuaJIT compilation
if (NOT DEFINED DISABLE_LUA)
    set(LUAJIT_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src/lua51.lib)

    if (NOT EXISTS ${LUAJIT_LIB_PATH})
        message(STATUS "LuaJIT library not found, compiling...")

        # Find Visual Studio installation
        find_program(VSWHERE_PATH "vswhere.exe"
                PATHS "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer"
                NO_DEFAULT_PATH)

        if (VSWHERE_PATH)
            execute_process(
                    COMMAND ${VSWHERE_PATH} -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
                    OUTPUT_VARIABLE VS_INSTALL_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            if (VS_INSTALL_PATH)
                # Convert to native path and escape properly
                file(TO_NATIVE_PATH "${VS_INSTALL_PATH}/VC/Auxiliary/Build/vcvars32.bat" VCVARS_PATH)

                execute_process(
                        COMMAND cmd /c call "${VCVARS_PATH}" && "msvcbuild.bat static"
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src
                        RESULT_VARIABLE LUAJIT_BUILD_RESULT
                )
            else ()
                message(FATAL_ERROR "Visual Studio installation not found")
            endif ()
        else ()
            # Fallback: try common VS paths
            set(COMMON_VS_PATHS
                    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars32.bat"
                    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Auxiliary/Build/vcvars32.bat"
                    "$ENV{ProgramFiles}/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars32.bat"
                    "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars32.bat"
            )

            set(VCVARS_FOUND FALSE)
            foreach (VCVARS_PATH_RAW ${COMMON_VS_PATHS})
                if (EXISTS "${VCVARS_PATH_RAW}")
                    # Convert to native path
                    file(TO_NATIVE_PATH "${VCVARS_PATH_RAW}" VCVARS_PATH)

                    execute_process(
                            COMMAND cmd /c "call /" ${VCVARS_PATH}/" && msvcbuild.bat static"
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src
                            RESULT_VARIABLE LUAJIT_BUILD_RESULT
                    )
                    set(VCVARS_FOUND TRUE)
                    break()
                endif ()
            endforeach ()

            if (NOT VCVARS_FOUND)
                message(FATAL_ERROR "Visual Studio vcvars32.bat not found. Please run CMake from Visual Studio Command Prompt or install Visual Studio.")
            endif ()
        endif ()

        if (NOT LUAJIT_BUILD_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to build LuaJIT")
        endif ()

        if (NOT EXISTS ${LUAJIT_LIB_PATH})
            message(FATAL_ERROR "LuaJIT build completed but library not found")
        endif ()

        message(STATUS "LuaJIT compiled successfully")
    else ()
        message(STATUS "LuaJIT library found, skipping compilation")
    endif ()
    # link luajit
    add_library(luajit_lib STATIC IMPORTED)
    set_target_properties(luajit_lib PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src/lua51.lib
    )
    target_include_directories(luajit_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/External/LuaJIT/src)
endif ()
