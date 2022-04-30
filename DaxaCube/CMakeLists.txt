
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaCube ${SOURCES})

target_link_libraries(DaxaCube PUBLIC daxa::daxa)
