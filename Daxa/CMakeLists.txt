
file(GLOB_RECURSE DAXA_SOURCES "*.cpp" "*.c")
add_library(Daxa STATIC ${DAXA_SOURCES})

find_package(glslang CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

target_link_libraries(Daxa PUBLIC
	HLSL SPIRV glslang OGLCompiler
	SDL2::SDL2
	spirv-cross-reflect
	unofficial::vulkan-memory-allocator::vulkan-memory-allocator
	Vulkan::Vulkan
	glm::glm
	imgui::imgui
)

target_compile_features(Daxa PUBLIC cxx_std_20)

target_include_directories(Daxa
	PUBLIC
		${STB_INCLUDE_DIRS}
		"dependencies"
		"include"
)

set_project_warnings(Daxa)