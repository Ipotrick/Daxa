
add_executable(test1 "test1.cpp")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(test1 PRIVATE daxa::daxa glfw)
