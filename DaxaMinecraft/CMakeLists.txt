
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaMinecraft ${SOURCES})

find_package(glm CONFIG REQUIRED)
target_link_libraries(DaxaMinecraft PUBLIC Daxa glm::glm)
target_include_directories(DaxaMinecraft PUBLIC "src")

# set_project_warnings(DaxaMinecraft)
