
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(Gabe ${SOURCES})

find_package(glm CONFIG REQUIRED)
target_link_libraries(Gabe PUBLIC Daxa glm::glm)
target_include_directories(Gabe PUBLIC "src")

# set_project_warnings(Gabe)
