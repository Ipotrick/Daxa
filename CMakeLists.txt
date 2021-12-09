cmake_minimum_required(VERSION 3.12)
project(sandbox)

find_package(Vulkan REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(glm CONFIG REQUIRED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    add_link_options(-fsanitize=address)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/fsanitize=address)
endif()

add_subdirectory(Daxa)
add_subdirectory(DaxaTriangle)
add_subdirectory(DaxaMinecraft)
