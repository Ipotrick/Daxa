
file(GLOB_RECURSE DAXA_SOURCES "*.cpp" "*.c")
add_library(Daxa STATIC ${DAXA_SOURCES})

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

set(NSIGHT_AFTERMATH_DIR "C:/dev/downloads/NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080")
if (DAXA_USE_NSIGHT_AFTERMATH AND EXISTS ${NSIGHT_AFTERMATH_DIR})
    add_library(nsight::aftermath SHARED IMPORTED)
    set_target_properties(nsight::aftermath PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${NSIGHT_AFTERMATH_DIR}/include"
        INTERFACE_LINK_DIRECTORIES "${NSIGHT_AFTERMATH_DIR}/lib/x64"

        IMPORTED_LOCATION ${NSIGHT_AFTERMATH_DIR}/lib/x64/GFSDK_Aftermath_Lib.x64.dll
        IMPORTED_IMPLIB   ${NSIGHT_AFTERMATH_DIR}/lib/x64/GFSDK_Aftermath_Lib.x64.lib
    )
    target_compile_definitions(nsight::aftermath INTERFACE USE_NSIGHT_AFTERMATH=1)
else()
    add_library(nsight_aftermath INTERFACE)
    add_library(nsight::aftermath ALIAS nsight_aftermath)
endif()

target_link_libraries(Daxa PUBLIC
	HLSL SPIRV glslang OGLCompiler
	unofficial::vulkan-memory-allocator::vulkan-memory-allocator
	Vulkan::Vulkan
	glm::glm
	dxc::dxc
	imgui::imgui
	glfw
	nsight::aftermath
	${SHADERC_LIBRARY_DIRS}
	${SHADERC_UTIL_LIBRARY_DIRS}
	${SHADERC_SPIRV_TOOLS_LIB}
	${SHADERC_SPIRV_TOOLS_LINK_LIB}
	${SHADERC_SPIRV_TOOLS_OPT_LIB}
	${SHADERC_SPIRV_TOOLS_REDUCE_LIB}
	${SHADERC_SPIRV_TOOLS_SHARED_LIB}
)

target_compile_features(Daxa PUBLIC cxx_std_20)

target_include_directories(Daxa
	PUBLIC
		${CGLTF_INCLUDE_DIRS}
		${STB_INCLUDE_DIRS}
		${SHADERC_INCLUDE_DIRS}
		"dependencies"
		"include"
	PRIVATE
		"dependencies/nsight_aftermath"
)

set_project_warnings(Daxa)