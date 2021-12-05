cmake_minimum_required(VERSION 3.12)
project(sandbox)

find_package(Vulkan REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_library(Daxa STATIC
    "Daxa/dependencies/spirv_reflect.cpp"
    "Daxa/dependencies/stb_image.cpp"
    "Daxa/dependencies/tiny_obj_loader.cpp"
    "Daxa/dependencies/VkBootstrap.cpp"
    "Daxa/dependencies/vk_mem_alloc.cpp"
    "Daxa/dependencies/vulkanhelper.cpp"
    "Daxa/src/Application.cpp"
    "Daxa/src/Daxa.cpp"
    "Daxa/src/DaxaApplicationOLD.cpp"
    "Daxa/src/platform/Scancodes.cpp"
    "Daxa/src/platform/Window.cpp"
    "Daxa/src/rendering/api_abstration/Buffer.cpp"
    "Daxa/src/rendering/api_abstration/CommandList.cpp"
    "Daxa/src/rendering/api_abstration/BindingSet.cpp"
    "Daxa/src/rendering/api_abstration/DescriptorSetLayoutCache.cpp"
    "Daxa/src/rendering/api_abstration/Device.cpp"
    "Daxa/src/rendering/api_abstration/Image.cpp"
    "Daxa/src/rendering/api_abstration/Pipeline.cpp"
    "Daxa/src/rendering/api_abstration/RenderWindow.cpp"
    "Daxa/src/rendering/api_abstration/ShaderModule.cpp"
    "Daxa/src/rendering/api_abstration/TimelineSemaphore.cpp"
    "Daxa/src/rendering/CoordSystem2d.cpp"
    "Daxa/src/rendering/Renderer.cpp"
    "Daxa/src/threading/Jobs.cpp"
    "Daxa/src/util/UUID.cpp"
)

target_compile_features(Daxa PUBLIC cxx_std_20)
target_link_libraries(Daxa PUBLIC
    Vulkan::Vulkan
    HLSL SPIRV glslang OGLCompiler
    SDL2::SDL2 SDL2::SDL2main
)
target_include_directories(Daxa PUBLIC "Daxa/include" "Daxa/dependencies")

add_executable(DaxaTriangle "DaxaTriangle/DaxaTriangle.cpp")
target_link_libraries(DaxaTriangle PUBLIC Daxa)

add_subdirectory(DaxaMinecraft)
