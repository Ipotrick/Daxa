
file(GLOB_RECURSE DAXA_SOURCES "*.cpp" "*.c")
add_library(daxa STATIC ${DAXA_SOURCES})
add_library(daxa::daxa ALIAS daxa)

find_library(SHADERC_LIBRARY_DIRS shaderc)
find_library(SHADERC_UTIL_LIBRARY_DIRS shaderc_util)
find_library(SHADERC_SPIRV_TOOLS_LIB SPIRV-Tools)
find_library(SHADERC_SPIRV_TOOLS_LINK_LIB SPIRV-Tools-link)
find_library(SHADERC_SPIRV_TOOLS_OPT_LIB SPIRV-Tools-opt)
find_library(SHADERC_SPIRV_TOOLS_REDUCE_LIB SPIRV-Tools-reduce)
find_library(SHADERC_SPIRV_TOOLS_SHARED_LIB SPIRV-Tools-shared)
find_path(SHADERC_INCLUDE_DIRS "shaderc/shaderc.h")

find_package(glslang CONFIG REQUIRED)
find_package(dxc CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_path(CGLTF_INCLUDE_DIRS "cgltf.h")

target_link_libraries(daxa PUBLIC
	HLSL SPIRV glslang OGLCompiler
	unofficial::vulkan-memory-allocator::vulkan-memory-allocator
	Vulkan::Vulkan
	glm::glm
	dxc::dxc
	imgui::imgui
	glfw
	$<BUILD_INTERFACE:${SHADERC_LIBRARY_DIRS}>
	$<BUILD_INTERFACE:${SHADERC_UTIL_LIBRARY_DIRS}>
	$<BUILD_INTERFACE:${SHADERC_SPIRV_TOOLS_LIB}>
	$<BUILD_INTERFACE:${SHADERC_SPIRV_TOOLS_LINK_LIB}>
	$<BUILD_INTERFACE:${SHADERC_SPIRV_TOOLS_OPT_LIB}>
	$<BUILD_INTERFACE:${SHADERC_SPIRV_TOOLS_REDUCE_LIB}>
	$<BUILD_INTERFACE:${SHADERC_SPIRV_TOOLS_SHARED_LIB}>
)

target_compile_features(daxa PUBLIC cxx_std_20)

target_include_directories(daxa
	PUBLIC
		$<BUILD_INTERFACE:${CGLTF_INCLUDE_DIRS}>
		$<BUILD_INTERFACE:${STB_INCLUDE_DIRS}>
		$<BUILD_INTERFACE:${SHADERC_INCLUDE_DIRS}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/dependencies>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/dependencies/nsight_aftermath>
)

# find_library(SHADERC_LIBRARY_DIRS shaderc)
# find_library(SHADERC_UTIL_LIBRARY_DIRS shaderc_util)
# find_library(SHADERC_SPIRV_TOOLS_LIB SPIRV-Tools)
# find_library(SHADERC_SPIRV_TOOLS_LINK_LIB SPIRV-Tools-link)
# find_library(SHADERC_SPIRV_TOOLS_OPT_LIB SPIRV-Tools-opt)
# find_library(SHADERC_SPIRV_TOOLS_REDUCE_LIB SPIRV-Tools-reduce)
# find_library(SHADERC_SPIRV_TOOLS_SHARED_LIB SPIRV-Tools-shared)
# find_path(SHADERC_INCLUDE_DIRS "shaderc/shaderc.h")

# find_package(glslang CONFIG REQUIRED)
# find_package(dxc CONFIG REQUIRED)
# find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
# find_package(Vulkan REQUIRED)
# find_package(glm CONFIG REQUIRED)
# find_package(imgui CONFIG REQUIRED)
# find_package(glfw3 CONFIG REQUIRED)
# find_path(STB_INCLUDE_DIRS "stb.h")
# find_path(CGLTF_INCLUDE_DIRS "cgltf.h")

# target_include_directories(daxa::daxa
# 	INTERFACE
# 		$<BUILD_INTERFACE:${CGLTF_INCLUDE_DIRS}>
# 		$<BUILD_INTERFACE:${STB_INCLUDE_DIRS}>
# 		$<BUILD_INTERFACE:${SHADERC_INCLUDE_DIRS}>
# )

set_project_warnings(daxa)

# Packaging
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
file(WRITE ${CMAKE_BINARY_DIR}/config.cmake.in [=[
@PACKAGE_INIT@
include(${CMAKE_CURRENT_LIST_DIR}/daxa-targets.cmake)
check_required_components(daxa)
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
