cmake_minimum_required(VERSION 3.21)

include(cmake/warnings.cmake)

project(Daxa)

add_subdirectory("Daxa")
# add_subdirectory("DaxaCube")
# add_subdirectory("DaxaTriangle")
add_subdirectory("DaxaComputeMandelbrot")
# add_subdirectory("DaxaDearImGui")
add_subdirectory("DaxaMeshview")

add_subdirectory("DaxaMinecraft")

add_subdirectory("Samples")
