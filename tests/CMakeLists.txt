
function(DAXA_CREATE_TEST)
    set(OPT_ARGS BRUH)
    set(SNG_ARGS BRUH2)
    set(MUL_ARGS FOLDER LIBS)
    cmake_parse_arguments(PARSE_ARGV 0 DCT "${OPT_ARGS}" "${SNG_ARGS}" "${MUL_ARGS}")

    list(JOIN DCT_FOLDER "_" FOLDER_NAME)
    list(JOIN DCT_FOLDER "/" FOLDER_PATH)

    add_executable(
        daxa_test_${FOLDER_NAME}
        "${FOLDER_PATH}/main.cpp"
    )
    target_link_libraries(daxa_test_${FOLDER_NAME} PRIVATE daxa::daxa ${DCT_LIBS})
    target_include_directories(daxa_test_${FOLDER_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/0_common/shared")
    target_compile_definitions(daxa_test_${FOLDER_NAME} PRIVATE DAXA_SAMPLE_PATH="${CMAKE_CURRENT_LIST_DIR}/${FOLDER_PATH}")

    if(DAXA_TESTS_DISABLE_WINDOWS_CONSOLE)
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            target_link_options(daxa_test_${FOLDER_NAME}
                PRIVATE "-Wl,/ENTRY:mainCRTStartup,/SUBSYSTEM:WINDOWS"
            )
        endif()
    endif()
endfunction()

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h" REQUIRED)
find_path(CGLTF_INCLUDE_DIRS "cgltf.h" REQUIRED)

add_library(stb_interface INTERFACE)
target_include_directories(stb_interface INTERFACE ${STB_INCLUDE_DIRS})
add_library(cgltf_interface STATIC "0_common/cgltf_impl.cpp")
target_include_directories(cgltf_interface PUBLIC ${CGLTF_INCLUDE_DIRS})
target_compile_definitions(cgltf_interface PUBLIC _CRT_SECURE_NO_WARNINGS=1)

DAXA_CREATE_TEST(
    FOLDER 1_setup 1_window
    LIBS glfw
)

DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 1_context
    LIBS
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 2_device
    LIBS
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 3_command_list
    LIBS
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 4_synchronization
    LIBS
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 5_swapchain
    LIBS glfw
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 6_task_list
    LIBS glfw
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 7_pipeline_manager
    LIBS
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 8_mem
    LIBS glfw
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 9_shader_integration
    LIBS glfw
)

DAXA_CREATE_TEST(
    FOLDER 3_samples 0_rectangle_cutting
    LIBS glfw
)
DAXA_CREATE_TEST(
    FOLDER 3_samples 1_mandelbrot
    LIBS glfw
)
DAXA_CREATE_TEST(
    FOLDER 3_samples 2_short_code_triangle
    LIBS glfw
)
DAXA_CREATE_TEST(
    FOLDER 3_samples 3_hello_triangle_compute
    LIBS glfw
)
#DAXA_CREATE_TEST(
#    FOLDER 3_samples 4_raster
#    LIBS glfw stb_interface
#)
DAXA_CREATE_TEST(
    FOLDER 3_samples 5_boids
    LIBS glfw
)
# DAXA_CREATE_TEST(
#     FOLDER 3_samples 6_sponza
#     LIBS glfw cgltf_interface
# )
if(DAXA_ENABLE_UTILS_FSR2)
    # DAXA_CREATE_TEST(
    #     FOLDER 3_samples 7_FSR2
    #     LIBS glfw stb_interface
    # )
endif()

DAXA_CREATE_TEST(
    FOLDER 4_hello_daxa 1_pink_screen
    LIBS glfw stb_interface
)

DAXA_CREATE_TEST(
    FOLDER 4_hello_daxa 2_triangle
    LIBS glfw stb_interface
)
