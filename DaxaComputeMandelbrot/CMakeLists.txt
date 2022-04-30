
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaComputeMandelbrot ${SOURCES})

target_link_libraries(DaxaComputeMandelbrot PUBLIC daxa::daxa)
