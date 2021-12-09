
add_executable(DaxaMinecraft "main.cpp")
target_link_libraries(DaxaMinecraft PUBLIC Daxa glm::glm)
target_include_directories(DaxaMinecraft PRIVATE ${STB_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR})
