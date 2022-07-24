cmake_minimum_required(VERSION 3.12)
include(cmake/warnings.cmake)

if(DAXA_ENABLE_UTILS)
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
)

add_library(daxa::daxa ALIAS daxa)

target_compile_features(daxa PUBLIC cxx_std_20)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(daxa PRIVATE /std:c++latest)
endif()

find_package(dxc CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(volk CONFIG REQUIRED)

if(DAXA_ENABLE_UTILS)
    find_package(imgui CONFIG REQUIRED)
    target_link_libraries(daxa
        PUBLIC
        imgui::imgui
    )
endif()

target_link_libraries(daxa
    PUBLIC
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    Vulkan::Vulkan
    volk::volk
    dxc::dxc
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

find_package(dxc CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(volk CONFIG REQUIRED)
]=])

if(DAXA_ENABLE_UTILS)
    file(APPEND ${CMAKE_BINARY_DIR}/config.cmake.in [=[
find_package(imgui CONFIG REQUIRED)
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
