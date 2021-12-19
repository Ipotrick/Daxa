
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaDeferred ${SOURCES})

target_link_libraries(DaxaDeferred PUBLIC Daxa)
