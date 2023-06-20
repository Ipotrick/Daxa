cmake_minimum_required(VERSION 3.21)

include(cmake/warnings.cmake)

if(DAXA_ENABLE_UTILS_FSR2)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-fsr2")
endif()
if(DAXA_ENABLE_UTILS_IMGUI)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-imgui")
endif()
if(DAXA_ENABLE_UTILS_MEM)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-mem")
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-pipeline-manager-glslang")
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_DXC)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-pipeline-manager-dxc")
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-pipeline-manager-spirv-validation")
endif()
if(DAXA_ENABLE_UTILS_TASK_LIST)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-task-list")
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
    "src/impl_memory_block.cpp"

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
find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

include(GNUInstallDirs)

target_compile_definitions(daxa
    PUBLIC
    $<BUILD_INTERFACE:DAXA_SHADER_INCLUDE_DIR="${CMAKE_CURRENT_LIST_DIR}/include">
    DAXA_SHADERLANG_GLSL=1
    DAXA_SHADERLANG_HLSL=2
)

if(DAXA_ENABLE_UTILS_FSR2)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_FSR2=true
    )
    find_package(fsr2 CONFIG REQUIRED)
    target_link_libraries(daxa
        PUBLIC
        fsr2::ffx_fsr2_api
        fsr2::ffx_fsr2_api_vk
    )
endif()
if(DAXA_ENABLE_UTILS_IMGUI)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_IMGUI=true
    )
    find_package(imgui CONFIG REQUIRED)
    target_link_libraries(daxa
        PUBLIC
        imgui::imgui
    )
endif()
if(DAXA_ENABLE_UTILS_MEM)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_MEM=true
    )
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG=true
    )
    find_package(glslang CONFIG REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(daxa
        PUBLIC
        glslang::glslang
        glslang::SPIRV
        glslang::HLSL
    )
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_DXC)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_DXC=true
    )
    find_package(dxc CONFIG REQUIRED)
    target_link_libraries(daxa
        PUBLIC
        dxc::dxc
    )
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION=true
    )
    find_package(SPIRV-Tools CONFIG REQUIRED)
    target_link_libraries(daxa
        PUBLIC
        SPIRV-Tools-static
    )
endif()
if(DAXA_ENABLE_UTILS_TASK_LIST)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_TASK_LIST=true
    )
endif()

target_link_libraries(daxa
    PUBLIC
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    Vulkan::Vulkan
    fmt::fmt
    glfw
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
file(WRITE ${CMAKE_BINARY_DIR}/config.cmake.in [=[
@PACKAGE_INIT@
include(${CMAKE_CURRENT_LIST_DIR}/daxa-targets.cmake)
check_required_components(daxa)

get_target_property(DAXA_PREV_DEFINITIONS daxa::daxa INTERFACE_COMPILE_DEFINITIONS)
set_target_properties(daxa::daxa PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "${DAXA_PREV_DEFINITIONS};DAXA_SHADER_INCLUDE_DIR=\"${PACKAGE_PREFIX_DIR}/include\""
)

find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
]=])

if(DAXA_ENABLE_UTILS_FSR2)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(fsr2 CONFIG REQUIRED)
]=])
endif()
if(DAXA_ENABLE_UTILS_IMGUI)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(imgui CONFIG REQUIRED)
]=])
endif()
if(DAXA_ENABLE_UTILS_MEM)
# No package management work to do
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(glslang CONFIG REQUIRED)
find_package(Threads REQUIRED)
]=])
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_DXC)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(dxc CONFIG REQUIRED)
]=])
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(SPIRV-Tools CONFIG REQUIRED)
]=])
endif()
if(DAXA_ENABLE_UTILS_TASK_LIST)
# No package management work to do
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
