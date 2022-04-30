
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaDearImGui ${SOURCES})

target_link_libraries(DaxaDearImGui PUBLIC daxa::daxa)
