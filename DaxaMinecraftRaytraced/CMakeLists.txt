
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaMinecraftRaytraced ${SOURCES})

find_package(glm CONFIG REQUIRED)
target_link_libraries(DaxaMinecraftRaytraced PUBLIC Daxa glm::glm)
target_include_directories(DaxaMinecraftRaytraced PUBLIC "src")

# set_project_warnings(DaxaMinecraftRaytraced)
