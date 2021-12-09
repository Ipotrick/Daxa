
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaTriangle ${SOURCES})

target_link_libraries(DaxaTriangle PUBLIC Daxa)
