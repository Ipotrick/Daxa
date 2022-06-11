
add_executable(DaxaMinecraft
    "src/main.cpp"
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(DaxaMinecraft PUBLIC daxa::daxa glm::glm)
target_include_directories(DaxaMinecraft PUBLIC "src")

# target_link_options(DaxaMinecraft PRIVATE -Wl,/SUBSYSTEM:Windows,/ENTRY:mainCRTStartup)

set_project_warnings(DaxaMinecraft)
