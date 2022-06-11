
file(GLOB_RECURSE DAXA_SOURCES "*.cpp" "*.c")
add_library(daxa STATIC ${DAXA_SOURCES})
add_library(daxa::daxa ALIAS daxa)

find_package(dxc CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_path(CGLTF_INCLUDE_DIRS "cgltf.h")

target_link_libraries(daxa PUBLIC
	unofficial::vulkan-memory-allocator::vulkan-memory-allocator
	Vulkan::Vulkan
	glm::glm
	fmt::fmt
	dxc::dxc
	imgui::imgui
	glfw
)

target_compile_features(daxa PUBLIC cxx_std_20)

target_include_directories(daxa
	PUBLIC
		$<BUILD_INTERFACE:${CGLTF_INCLUDE_DIRS}>
		$<BUILD_INTERFACE:${STB_INCLUDE_DIRS}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/dependencies>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_project_warnings(daxa)

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
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

]=])
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
install(DIRECTORY ${PROJECT_SOURCE_DIR}/Daxa/include/ TYPE INCLUDE)
