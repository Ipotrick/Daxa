cmake_minimum_required(VERSION 3.12)

include(cmake/warnings.cmake)

project(daxa VERSION 0.1.0)

add_subdirectory("Daxa")

if(DAXA_BUILD_SAMPLES)
    # add_subdirectory("DaxaCube")
    # add_subdirectory("DaxaTriangle")
    add_subdirectory("DaxaComputeMandelbrot")
    # add_subdirectory("DaxaDearImGui")
    add_subdirectory("DaxaMeshview")

    add_subdirectory("DaxaMinecraft")

    add_subdirectory("Samples")
endif()
