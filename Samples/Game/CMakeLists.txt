
add_executable(DaxaSamples_Game
    "src/main.cpp"
)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(DaxaSamples_Game PUBLIC Daxa glm::glm fmt::fmt)
target_include_directories(DaxaSamples_Game PUBLIC "src")

# target_link_options(DaxaSamples_Game PRIVATE -Wl,/SUBSYSTEM:Windows,/ENTRY:mainCRTStartup)

set_project_warnings(DaxaSamples_Game)
