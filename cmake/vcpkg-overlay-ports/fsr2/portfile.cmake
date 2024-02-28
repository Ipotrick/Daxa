vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/GPUOpen-Effects/FidelityFX-FSR2
    REF 1680d1edd5c034f88ebbbb793d8b88f8842cf804
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    directx12 WITH_DX12
    vulkan WITH_VULKAN
)
set(FSR2_DEFINES)

file(WRITE "${SOURCE_PATH}/src/ffx-fsr2-api/CMakeLists.txt" [==[
cmake_minimum_required(VERSION 3.15)
project(ffx_fsr2_api VERSION 2.2.0)
set(CMAKE_DEBUG_POSTFIX d)
option(FFX_FSR2_API_DX12 "Build FSR 2.0 DX12 backend" ON)
option(FFX_FSR2_API_VK "Build FSR 2.0 Vulkan backend" ON)
set(FSR2_AUTO_COMPILE_SHADERS ON CACHE BOOL "Compile shaders automatically as a prebuild step.")

if(CMAKE_GENERATOR STREQUAL "Ninja")
    set(USE_DEPFILE TRUE)
else()
    set(USE_DEPFILE FALSE)
endif()

if(CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019")
    set(FSR2_VS_VERSION 2019)
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "x64" OR CMAKE_EXE_LINKER_FLAGS STREQUAL "/machine:x64")
    else()
        message(FATAL_ERROR "Unsupported target platform - only supporting x64 and Win32 currently")
    endif()
    # Embed PDBs in the debug versions of the libs
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
endif()


# Write both debug and release versions of the static libs to the /lib folder as they are uniquely named
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_HOME_DIRECTORY}/bin/ffx_fsr2_api/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_HOME_DIRECTORY}/bin/ffx_fsr2_api/)

add_compile_definitions(_UNICODE)
add_compile_definitions(UNICODE)
#add_compile_definitions(FSR2_VERSION_MAJOR=0)
#add_compile_definitions(FSR2_VERSION_MINOR=1)
#add_compile_definitions(FSR2_VERSION_PATCH=0)

if(FSR2_VS_VERSION STREQUAL 2015)
    message(NOTICE "Forcing the SDK path for VS 2015")
    set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.18362.0")
endif()

set(FFX_SC_EXECUTABLE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/sc/FidelityFX_SC.exe)

set(FFX_SC_BASE_ARGS
    -reflection -deps=gcc -DFFX_GPU=1
    # Only reprojection is to do half for now
    -DFFX_FSR2_OPTION_UPSAMPLE_SAMPLERS_USE_DATA_HALF=0
    -DFFX_FSR2_OPTION_ACCUMULATE_SAMPLERS_USE_DATA_HALF=0
    -DFFX_FSR2_OPTION_REPROJECT_SAMPLERS_USE_DATA_HALF=1
    -DFFX_FSR2_OPTION_POSTPROCESSLOCKSTATUS_SAMPLERS_USE_DATA_HALF=0
    # Upsample uses lanczos approximation
    -DFFX_FSR2_OPTION_UPSAMPLE_USE_LANCZOS_TYPE=2
    )

set(FFX_SC_PERMUTATION_ARGS
    # Reproject can use either reference lanczos or LUT
    -DFFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE={0,1}
    -DFFX_FSR2_OPTION_HDR_COLOR_INPUT={0,1}
    -DFFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS={0,1}
    -DFFX_FSR2_OPTION_JITTERED_MOTION_VECTORS={0,1}
    -DFFX_FSR2_OPTION_INVERTED_DEPTH={0,1}
    -DFFX_FSR2_OPTION_APPLY_SHARPENING={0,1})
 
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

if (FSR2_BUILD_AS_DLL)
    add_library(ffx_fsr2_api      SHARED ${SOURCES})
else()
    add_library(ffx_fsr2_api      STATIC ${SOURCES})
endif()

if(FFX_FSR2_API_DX12)
    message("Will build FSR2 library: DX12 backend")
    add_subdirectory(dx12)
endif()

if(FFX_FSR2_API_VK)
    message("Will build FSR2 library: Vulkan backend")
    add_subdirectory(vk)
endif()

# api
source_group("source"  FILES ${SOURCES})

set_source_files_properties(${SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
# Packaging
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
file(WRITE ${CMAKE_BINARY_DIR}/config.cmake.in [=[
@PACKAGE_INIT@
include(${CMAKE_CURRENT_LIST_DIR}/fsr2-targets.cmake)
check_required_components(ffx_fsr2_api)
]=])

configure_package_config_file(${CMAKE_BINARY_DIR}/config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/fsr2-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/fsr2
    NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/fsr2-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/fsr2-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/fsr2-config-version.cmake
    DESTINATION
    ${CMAKE_INSTALL_DATADIR}/fsr2)
install(TARGETS ffx_fsr2_api ffx_fsr2_api_vk EXPORT fsr2-targets)
install(EXPORT fsr2-targets DESTINATION ${CMAKE_INSTALL_DATADIR}/fsr2 NAMESPACE fsr2::)
# TODO: Install only the files that are necessary:
#   ffx_assert.h
#   ffx_error.h
#   ffx_util.h
#   ffx_types.h
#   ffx_fsr2.h
#   ffx_fsr2_interface.h
#   ffx_fsr2_common.h
#   ffx_fsr2_resources.h
install(DIRECTORY ${PROJECT_SOURCE_DIR} TYPE INCLUDE FILES_MATCHING PATTERN "*.h")
]==])

if(VCPKG_TARGET_IS_LINUX)
    file(READ "${SOURCE_PATH}/src/ffx-fsr2-api/ffx_util.h" ffx_util)
    string(APPEND ffx_util [=[
#include <wchar.h>
#include <locale>
#define _countof(x) (sizeof(x) / sizeof(x[0]))
#define wcscpy_s wcscpy
]=])
    file(WRITE "${SOURCE_PATH}/src/ffx-fsr2-api/ffx_util.h" "${ffx_util}")

    file(READ "${SOURCE_PATH}/src/ffx-fsr2-api/ffx_types.h" ffx_types)
    string(REGEX REPLACE "pragma once" "pragma once\n#include <stddef.h>" ffx_types "${ffx_types}")
    string(REGEX REPLACE "__declspec.dllexport." " " ffx_types "${ffx_types}")
    file(WRITE "${SOURCE_PATH}/src/ffx-fsr2-api/ffx_types.h" "${ffx_types}")
endif()

file(WRITE "${SOURCE_PATH}/src/ffx-fsr2-api/vk/CMakeLists.txt" [==[
if(NOT ${FFX_FSR2_API_VK})
    return()
endif()

set(FFX_SC_VK_BASE_ARGS
    -compiler=glslang -e main --target-env vulkan1.1 -S comp -Os -DFFX_GLSL=1)

file(GLOB SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/*.glsl")

set(PASS_SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_tcr_autogen_pass.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_autogen_reactive_pass.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_accumulate_pass.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_compute_luminance_pyramid_pass.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_depth_clip_pass.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_lock_pass.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_reconstruct_previous_depth_pass.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ffx_fsr2_rcas_pass.glsl)    

file(GLOB_RECURSE VK
    "${CMAKE_CURRENT_SOURCE_DIR}/../ffx_assert.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if (FSR2_BUILD_AS_DLL)
    add_library(ffx_fsr2_api_vk SHARED ${VK})
    target_link_libraries(ffx_fsr2_api_vk LINK_PUBLIC Vulkan::Vulkan)
else()
    add_library(ffx_fsr2_api_vk STATIC ${VK})
endif()

find_package(Vulkan REQUIRED)
target_link_libraries(ffx_fsr2_api_vk PUBLIC Vulkan::Vulkan)

target_include_directories(ffx_fsr2_api_vk PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../shaders/vk>)
target_include_directories(ffx_fsr2_api_vk PUBLIC ${Vulkan_INCLUDE_DIR})

if (UNIX)
file(DOWNLOAD "https://github.com/GabeRundlett/fsr2-precompiled-shaders/archive/refs/tags/0.2.0.zip" "${CMAKE_CURRENT_BINARY_DIR}/shaders.zip")
file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders.zip" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/..")
file(COPY "${CMAKE_CURRENT_BINARY_DIR}/../fsr2-precompiled-shaders-0.2.0/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/..")
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/../fsr2-precompiled-shaders-0.2.0")
else()
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../shaders/vk)

if (FSR2_AUTO_COMPILE_SHADERS)
    set(FFX_SC_DEPENDENT_TARGET ffx_fsr2_api_vk)
else()
    set(FFX_SC_DEPENDENT_TARGET ffx_fsr2_api_vk_shaders)
    add_custom_target(${FFX_SC_DEPENDENT_TARGET})
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20.0") 
cmake_policy(SET CMP0116 OLD)
endif()
get_filename_component(PASS_SHADER_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../shaders/vk ABSOLUTE)
foreach(PASS_SHADER ${PASS_SHADERS})
    get_filename_component(PASS_SHADER_FILENAME ${PASS_SHADER} NAME_WE)
    get_filename_component(PASS_SHADER_TARGET ${PASS_SHADER} NAME_WLE)
    set(PERMUTATION_HEADER ${PASS_SHADER_OUTPUT_PATH}/${PASS_SHADER_TARGET}_permutations.h)

    # combine base and permutation args
    if (${PASS_SHADER_FILENAME} STREQUAL "ffx_fsr2_compute_luminance_pyramid_pass")
        # skip 16-bit permutations for the compute luminance pyramid pass
        set(FFX_SC_ARGS ${FFX_SC_BASE_ARGS} ${FFX_SC_VK_BASE_ARGS} ${FFX_SC_PERMUTATION_ARGS} -DFFX_HALF=0)  
    else()
        set(FFX_SC_ARGS ${FFX_SC_BASE_ARGS} ${FFX_SC_VK_BASE_ARGS} ${FFX_SC_PERMUTATION_ARGS} -DFFX_HALF={0,1})
    endif()

    if(USE_DEPFILE)
        add_custom_command(
            OUTPUT ${PERMUTATION_HEADER}
            COMMAND ${FFX_SC_EXECUTABLE} ${FFX_SC_ARGS} -name=${PASS_SHADER_FILENAME} -I${CMAKE_CURRENT_SOURCE_DIR}/shaders -output=${PASS_SHADER_OUTPUT_PATH} ${PASS_SHADER}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${PASS_SHADER}
            DEPFILE ${PERMUTATION_HEADER}.d
        )
        list(APPEND PERMUTATION_OUTPUTS ${PERMUTATION_HEADER})
    else()
        add_custom_command(
            OUTPUT ${PERMUTATION_HEADER}
            COMMAND ${FFX_SC_EXECUTABLE} ${FFX_SC_ARGS} -name=${PASS_SHADER_FILENAME} -I${CMAKE_CURRENT_SOURCE_DIR}/shaders -output=${PASS_SHADER_OUTPUT_PATH} ${PASS_SHADER}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${PASS_SHADER}
        )
        list(APPEND PERMUTATION_OUTPUTS ${PERMUTATION_HEADER})
    endif()
endforeach(PASS_SHADER)

add_custom_target(shader_permutations_vk DEPENDS ${PERMUTATION_OUTPUTS})
add_dependencies(${FFX_SC_DEPENDENT_TARGET} shader_permutations_vk)

source_group("source"  FILES ${VK})
source_group("shaders" FILES ${SHADERS})
endif()
]==])

if(WITH_DX12)
    list(APPEND FSR2_DEFINES "-DFFX_FSR2_API_DX12=ON")
    list(APPEND FSR2_DEFINES "-DFFX_FSR2_API_VK=OFF")
endif()

if(WITH_VULKAN)
    list(APPEND FSR2_DEFINES "-DFFX_FSR2_API_DX12=OFF")
    list(APPEND FSR2_DEFINES "-DFFX_FSR2_API_VK=ON")
endif()

vcpkg_configure_cmake(
    SOURCE_PATH "${SOURCE_PATH}/src/ffx-fsr2-api"
    PREFER_NINJA
    OPTIONS ${FSR2_DEFINES}
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/include/ffx-fsr2-api/bin/ffx_fsr2_api")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/include/ffx-fsr2-api/bin")
file(INSTALL "${SOURCE_PATH}/LICENSE.txt"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
)
