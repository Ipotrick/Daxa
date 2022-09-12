cmake_minimum_required(VERSION 3.21)

include(cmake/warnings.cmake)

if(DAXA_ENABLE_UTILS)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils")
endif()
if(DAXA_ENABLE_DXC)
    list(APPEND VCPKG_MANIFEST_FEATURES "dxc")
endif()
if(DAXA_ENABLE_GLSLANG)
    list(APPEND VCPKG_MANIFEST_FEATURES "glslang")
endif()
if(DAXA_ENABLE_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(daxa VERSION 1.0.0)

if(DAXA_ENABLE_STATIC_ANALYSIS)
    set(CPPCHECK_TEMPLATE "gcc")
    find_program(CPPCHECK cppcheck)
    find_program(CLANG_TIDY clang-tidy)
    set(CMAKE_CXX_CPPCHECK
        ${CPPCHECK}
        --template=${CPPCHECK_TEMPLATE}
        --enable=style,performance,warning,portability
        --inline-suppr
        --suppress=internalAstError
        --suppress=unmatchedSuppression
        --inconclusive)
    set(CMAKE_CXX_CPPCHECK
        ${CPPCHECK}
        --template=${CPPCHECK_TEMPLATE}
        ${CPPCHECK_OPTIONS})
endif()

add_library(daxa STATIC
    "src/impl_device.cpp"
    "src/impl_context.cpp"
    "src/impl_core.cpp"
    "src/impl_pipeline.cpp"
    "src/impl_swapchain.cpp"
    "src/impl_command_list.cpp"
    "src/impl_gpu_resources.cpp"
    "src/impl_semaphore.cpp"
    "src/impl_dependencies.cpp"

    "src/utils/impl_task_list.cpp"
    "src/utils/impl_imgui.cpp"
    "src/utils/impl_fsr2.cpp"
    "src/utils/impl_mem.cpp"
)

add_library(daxa::daxa ALIAS daxa)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(daxa PUBLIC /std:c++latest)
else()
    target_compile_features(daxa PUBLIC cxx_std_20)
endif()

find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(volk CONFIG REQUIRED)

if(DAXA_ENABLE_GLSLANG)
    find_package(glslang CONFIG REQUIRED)

    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_GLSLANG=true
    )
    target_link_libraries(daxa
        PUBLIC
        HLSL SPIRV glslang OGLCompiler
    )
endif()

if(DAXA_ENABLE_DXC)
    find_package(dxc CONFIG REQUIRED)

    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_DXC=true
    )
    target_link_libraries(daxa
        PUBLIC
        dxc::dxc
    )
endif()


if(DAXA_ENABLE_UTILS)
    find_package(imgui CONFIG REQUIRED)
    find_package(fsr2 CONFIG REQUIRED)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS=true
    )
    target_link_libraries(daxa
        PUBLIC
        imgui::imgui
        fsr2::ffx_fsr2_api
        fsr2::ffx_fsr2_api_vk
    )
endif()

target_link_libraries(daxa
    PUBLIC
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    Vulkan::Vulkan
    volk::volk
)

target_include_directories(daxa
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_project_warnings(daxa)

if(DAXA_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

# Packaging
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
file(WRITE ${CMAKE_BINARY_DIR}/config.cmake.in [=[
@PACKAGE_INIT@
include(${CMAKE_CURRENT_LIST_DIR}/daxa-targets.cmake)
check_required_components(daxa)

find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(volk CONFIG REQUIRED)
]=])

if(DAXA_ENABLE_GLSLANG)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(glslang CONFIG REQUIRED)
]=])
endif()

if(DAXA_ENABLE_DXC)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(dxc CONFIG REQUIRED)
]=])
endif()

if(DAXA_ENABLE_UTILS)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(imgui CONFIG REQUIRED)
find_package(fsr2 CONFIG REQUIRED)
]=])
endif()

configure_package_config_file(${CMAKE_BINARY_DIR}/config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/daxa-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/daxa
    NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/daxa-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/daxa-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/daxa-config-version.cmake
    DESTINATION
    ${CMAKE_INSTALL_DATADIR}/daxa)
install(TARGETS daxa EXPORT daxa-targets)
install(EXPORT daxa-targets DESTINATION ${CMAKE_INSTALL_DATADIR}/daxa NAMESPACE daxa::)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ TYPE INCLUDE)
