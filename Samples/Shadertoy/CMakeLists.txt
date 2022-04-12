cmake_minimum_required(VERSION 3.21)

add_executable(DaxaSamples_Shadertoy
    "main.cpp"
)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(DaxaSamples_Shadertoy PUBLIC Daxa glm::glm fmt::fmt)

set_project_warnings(DaxaSamples_Shadertoy)
