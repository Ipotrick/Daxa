cmake_minimum_required(VERSION 3.21)

include(cmake/warnings.cmake)

if(DAXA_ENABLE_UTILS)
    if(DAXA_ENABLE_DXC)
        list(APPEND VCPKG_MANIFEST_FEATURES "dxc")
    endif()
    if(DAXA_ENABLE_GLSLANG)
        list(APPEND VCPKG_MANIFEST_FEATURES "glslang")
    endif()
    list(APPEND VCPKG_MANIFEST_FEATURES "utils")
endif()
if(DAXA_ENABLE_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(daxa VERSION 1.0.0)

if(DAXA_ENABLE_STATIC_ANALYSIS)
    set(CPPCHECK_TEMPLATE "gcc")
    find_program(CPPCHECK cppcheck)
    find_program(CLANG_TIDY clang-tidy)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK}
            --template=${CPPCHECK_TEMPLATE}
            --enable=style,performance,warning,portability
            --inline-suppr
            --suppress=cppcheckError
            --suppress=internalAstError
            --suppress=unmatchedSuppression
            --suppress=preprocessorErrorDirective
            --suppress=exceptThrowInDestructor
            --suppress=functionStatic
            --inconclusive)
    endif()
    if(CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY}
            --fix)
    endif()
else()
    set(CMAKE_UNITY_BUILD true)
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
    "src/impl_split_barrier.cpp"
    "src/impl_timeline_query.cpp"

    "src/utils/impl_task_list.cpp"
    "src/utils/impl_imgui.cpp"
    "src/utils/impl_fsr2.cpp"
    "src/utils/impl_mem.cpp"
    "src/utils/impl_pipeline_manager.cpp"
)

add_library(daxa::daxa ALIAS daxa)

target_compile_features(daxa PUBLIC cxx_std_20)

find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

target_compile_definitions(daxa
    PUBLIC
    DAXA_SHADERLANG_GLSL=1
    DAXA_SHADERLANG_HLSL=2
)

if(DAXA_ENABLE_UTILS)
    if(DAXA_ENABLE_GLSLANG)
        find_package(glslang CONFIG REQUIRED)
        find_package(Threads REQUIRED)

        target_compile_definitions(daxa
            PUBLIC
            DAXA_BUILT_WITH_GLSLANG=true
        )
        target_link_libraries(daxa
            PUBLIC
            glslang::glslang
            glslang::SPIRV
            glslang::HLSL
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
)

target_include_directories(daxa
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11)
    if(X11_FOUND)
        target_compile_definitions(daxa
            PUBLIC
            DAXA_BUILT_WITH_X11=true
        )
        target_link_libraries(daxa
            PUBLIC
            ${X11_LIBRARIES}
        )
    endif()

    find_package(Wayland)
    if(Wayland_FOUND)
        target_compile_definitions(daxa
            PUBLIC
            DAXA_BUILT_WITH_WAYLAND=true
        )
        target_link_libraries(daxa
            PUBLIC
            ${Wayland_LIBRARIES}
        )
    endif()
endif()

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
]=])

if(DAXA_ENABLE_UTILS)
    if(DAXA_ENABLE_GLSLANG)
        file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(glslang CONFIG REQUIRED)
find_package(Threads REQUIRED)
]=])
    endif()

    if(DAXA_ENABLE_DXC)
        file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(dxc CONFIG REQUIRED)
]=])
    endif()

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
