
function(DAXA_CREATE_TEST FOLDER SUBFOLDER)
    add_executable(
        daxa_test_${FOLDER}_${SUBFOLDER}
        "${FOLDER}/${SUBFOLDER}/main.cpp"
    )
    target_link_libraries(daxa_test_${FOLDER}_${SUBFOLDER} PRIVATE daxa::daxa glfw glm::glm)
    target_include_directories(daxa_test_${FOLDER}_${SUBFOLDER} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

    if(DAXA_TESTS_DISABLE_WINDOWS_CONSOLE)
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            target_link_options(daxa_test_${FOLDER}_${SUBFOLDER}
                PRIVATE "-Wl,/ENTRY:mainCRTStartup,/SUBSYSTEM:WINDOWS"
            )
        endif()
    endif()
endfunction()

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

DAXA_CREATE_TEST(1_setup 1_window)

DAXA_CREATE_TEST(2_daxa_api 1_context)
DAXA_CREATE_TEST(2_daxa_api 2_device)
DAXA_CREATE_TEST(2_daxa_api 3_command_list)
DAXA_CREATE_TEST(2_daxa_api 4_synchronization)
DAXA_CREATE_TEST(2_daxa_api 5_swapchain)

DAXA_CREATE_TEST(3_samples 0_playground)
DAXA_CREATE_TEST(3_samples 1_mandelbrot)
DAXA_CREATE_TEST(3_samples 2_hello_triangle)
DAXA_CREATE_TEST(3_samples 3_hello_triangle_compute)
DAXA_CREATE_TEST(3_samples 4_raster)
DAXA_CREATE_TEST(3_samples 5_boids)
