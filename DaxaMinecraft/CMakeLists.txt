
add_executable(DaxaMinecraft "main.cpp")

find_path(STB_INCLUDE_DIRS "stb.h")
find_package(glm CONFIG REQUIRED)

target_link_libraries(DaxaMinecraft PUBLIC Daxa glm::glm)
target_include_directories(DaxaMinecraft PRIVATE ${STB_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR})
