
add_executable(test1 "test1.cpp")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(test1 PRIVATE daxa::daxa glfw)

if (DAXA_TESTS_DISABLE_WINDOWS_CONSOLE)

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_link_options(test1 PRIVATE "-Wl,/ENTRY:mainCRTStartup,/SUBSYSTEM:WINDOWS")
    endif()

endif()
