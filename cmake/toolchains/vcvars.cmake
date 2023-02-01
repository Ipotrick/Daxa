function(msvc_inherit_from_vcvars)
    # vcvarsall.bat does not check if the PATH is already populated
    # with the necessary directories, and just naively appends them
    # to the existing PATH. Because of this, if this function gets
    # called more than a few times, it extends the PATH to exceed
    # a valid length and fails. Here I'm checking if the DevEnvDir
    # environment variable is set, and shortcutting this function
    # if so.
    # CMake will call the toolchain file multiple times when configuring
    # for the first time to do things like check if a compiler works
    # for a super simple file. Even though we check if we've set variables
    # in the cache to not re-call this function, the cache is NOT
    # populated until AFTER all the initial configuration is done,
    # therefore we must make sure that it's not a subsequent call from
    # checking the actual CMD environment
    execute_process(COMMAND cmd /c if defined DevEnvDir (exit 1) RESULT_VARIABLE CMD_RESULT)
    if(NOT CMD_RESULT EQUAL 0)
        return()
    endif()

    # Adapted from Modules/Platform/Windows-GNU.cmake
    set(VS_INSTALLER_PATHS "")
    set(VS_VERSIONS 17)
    foreach(VER ${VS_VERSIONS}) # change the first number to the largest supported version
        cmake_host_system_information(RESULT VS_DIR QUERY VS_${VER}_DIR)
        if(VS_DIR)
            list(APPEND VS_INSTALLER_PATHS "${VS_DIR}/VC/Auxiliary/Build")
        endif()
    endforeach()
    
    find_program(VCVARSALL_PATH NAMES vcvars64.bat vcvarsamd64.bat
        DOC "Visual Studio vcvarsamd64.bat"
        PATHS ${VS_INSTALLER_PATHS}
    )

    if(NOT EXISTS "${VCVARSALL_PATH}")
        message(FATAL_ERROR "Unknown VS version specified/no vcvarsall.bat detected")
    else()
        message("vcvars file at ${VCVARSALL_PATH}")
    endif()
    execute_process(COMMAND cmd /c echo {SET0} && set && echo {/SET0} && "${VCVARSALL_PATH}" x64 && echo {SET1} && set && echo {/SET1} OUTPUT_VARIABLE CMD_OUTPUT RESULT_VARIABLE CMD_RESULT)
    if(NOT CMD_RESULT EQUAL 0)
        message(FATAL_ERROR "command returned '${CMD_RESULT}'")
    endif()

    # Parse the command output
    set(REST "${CMD_OUTPUT}")
    string(FIND "${REST}" "{SET0}" BEG)
    string(SUBSTRING "${REST}" ${BEG} -1 REST)
    string(FIND "${REST}" "{/SET0}" END)
    string(SUBSTRING "${REST}" 0 ${END} SET0)
    string(SUBSTRING "${SET0}" 6 -1 SET0)
    string(FIND "${REST}" "{SET1}" BEG)
    string(SUBSTRING "${REST}" ${BEG} -1 REST)
    string(FIND "${REST}" "{/SET1}" END)
    string(SUBSTRING "${REST}" 0 ${END} SET1)
    string(SUBSTRING "${SET1}" 6 -1 SET1)
    string(REGEX MATCHALL "\n[0-9a-zA-Z_]*" SET0_VARS "${SET0}")
    list(TRANSFORM SET0_VARS STRIP)
    string(REGEX MATCHALL "\n[0-9a-zA-Z_]*" SET1_VARS "${SET1}")
    list(TRANSFORM SET1_VARS STRIP)

    function(_extract_from_set_command INPUT VARNAME OUTVAR_NAME)
        set(R "${INPUT}")
        string(FIND "${R}" "\n${VARNAME}=" B)
        if(B EQUAL -1)
            set(${OUTVAR_NAME} "" PARENT_SCOPE)
            return()
        endif()
        string(SUBSTRING "${R}" ${B} -1 R)
        string(SUBSTRING "${R}" 1 -1 R)
        string(FIND "${R}" "\n" E)
        string(SUBSTRING "${R}" 0 ${E} OUT_TEMP)
        string(LENGTH "${VARNAME}=" VARNAME_LEN)
        string(SUBSTRING "${OUT_TEMP}" ${VARNAME_LEN} -1 OUT_TEMP)
        set(${OUTVAR_NAME} "${OUT_TEMP}" PARENT_SCOPE)
    endfunction()
    set(CHANGED_VARS)
    # Run over all the vars in set1 (the set containing all the new environment vars)
    # and compare their values to their respective value in set0 (the value before
    # running vcvarsall)
    foreach(V ${SET1_VARS})
        _extract_from_set_command("${SET0}" ${V} V0)
        _extract_from_set_command("${SET1}" ${V} V1)
        if(NOT ("${V0}" STREQUAL "${V1}"))
            # if it is different, then we'll add it to the list
            list(APPEND CHANGED_VARS ${V})
            # and also we'll cache it as the value from set1.
            if(V STREQUAL "Path")
                string(REGEX REPLACE "\\\\" "/" V1 "${V1}")
            endif()
            set(MSVC_ENV_${V} "${V1}" CACHE STRING "")
        endif()
    endforeach()
    set(MSVC_ENV_VAR_NAMES ${CHANGED_VARS} CACHE STRING "")
endfunction()

if(NOT DEFINED MSVC_ENV_VAR_NAMES)
    msvc_inherit_from_vcvars()
endif()
