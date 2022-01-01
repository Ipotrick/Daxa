
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaMeshview ${SOURCES})

target_link_libraries(DaxaMeshview PUBLIC Daxa)
