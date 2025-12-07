
function(DAXA_CREATE_TEST)
    set(OPT_ARGS BRUH)
    set(SNG_ARGS BRUH2)
    set(MUL_ARGS FOLDER LIBS FEATURES)
    cmake_parse_arguments(PARSE_ARGV 0 DCT "${OPT_ARGS}" "${SNG_ARGS}" "${MUL_ARGS}")

    list(JOIN DCT_FOLDER "_" FOLDER_NAME)
    list(JOIN DCT_FOLDER "/" FOLDER_PATH)

    foreach(FEATURE_NAME ${DCT_FEATURES})
        if (NOT DAXA_ENABLE_${FEATURE_NAME})
            return()
        endif()
    endforeach()

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

    add_custom_command(TARGET daxa_test_${FOLDER_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:daxa_test_${FOLDER_NAME}> $<TARGET_FILE_DIR:daxa_test_${FOLDER_NAME}>
        COMMAND_EXPAND_LISTS
    )
endfunction()

function(DAXA_CREATE_TEST_C)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC literally can't compile C properly.
        return()
    endif()

    set(OPT_ARGS BRUH)
    set(SNG_ARGS BRUH2)
    set(MUL_ARGS FOLDER LIBS FEATURES)
    cmake_parse_arguments(PARSE_ARGV 0 DCT "${OPT_ARGS}" "${SNG_ARGS}" "${MUL_ARGS}")

    list(JOIN DCT_FOLDER "_" FOLDER_NAME)
    list(JOIN DCT_FOLDER "/" FOLDER_PATH)

    foreach(FEATURE_NAME ${DCT_FEATURES})
        if (NOT DAXA_ENABLE_${FEATURE_NAME})
            return()
        endif()
    endforeach()

    add_executable(
        daxa_test_c_${FOLDER_NAME}
        "${FOLDER_PATH}/main.c"
    )
    target_link_libraries(daxa_test_c_${FOLDER_NAME} PRIVATE daxa::daxa ${DCT_LIBS})
    target_include_directories(daxa_test_c_${FOLDER_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/0_common/shared")
    target_compile_definitions(daxa_test_c_${FOLDER_NAME} PRIVATE DAXA_SAMPLE_PATH="${CMAKE_CURRENT_LIST_DIR}/${FOLDER_PATH}")

    add_custom_command(TARGET daxa_test_c_${FOLDER_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:daxa_test_c_${FOLDER_NAME}> $<TARGET_FILE_DIR:daxa_test_c_${FOLDER_NAME}>
        COMMAND_EXPAND_LISTS
    )
endfunction()

# find_package(glfw3 CONFIG REQUIRED)

DAXA_CREATE_TEST(
    FOLDER 1_setup 1_window
    LIBS glfw
)

DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 1_instance
    LIBS
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 2_device
    LIBS
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 3_command_recorder
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
    FOLDER 2_daxa_api 6_task_graph
    LIBS glfw
    FEATURES
        UTILS_TASK_GRAPH
        UTILS_IMGUI
        UTILS_PIPELINE_MANAGER_GLSLANG
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 7_pipeline_manager
    LIBS
    FEATURES
        UTILS_PIPELINE_MANAGER_GLSLANG
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 8_mem
    LIBS glfw
    FEATURES
        UTILS_MEM
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 9_shader_integration
    LIBS glfw
    FEATURES
        UTILS_TASK_GRAPH
        UTILS_PIPELINE_MANAGER_GLSLANG
)
# DAXA_CREATE_TEST(
#     FOLDER 2_daxa_api 10_raytracing
#     LIBS glfw
# )
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 11_mesh_shader
    LIBS glfw
    FEATURES
        UTILS_TASK_GRAPH
        UTILS_PIPELINE_MANAGER_SLANG
)
DAXA_CREATE_TEST(
    FOLDER 2_daxa_api 12_async_queues
    LIBS glfw
    FEATURES
        UTILS_TASK_GRAPH
        UTILS_PIPELINE_MANAGER_SLANG
)

DAXA_CREATE_TEST(
    FOLDER 3_samples 0_rectangle_cutting
    LIBS glfw
    FEATURES
        UTILS_IMGUI
        UTILS_PIPELINE_MANAGER_GLSLANG
)
DAXA_CREATE_TEST(
    FOLDER 3_samples 1_mandelbrot
    LIBS glfw
    FEATURES
        UTILS_TASK_GRAPH
        UTILS_IMGUI
        UTILS_PIPELINE_MANAGER_GLSLANG
        UTILS_PIPELINE_MANAGER_SLANG
)
# DAXA_CREATE_TEST(
#     FOLDER 3_samples 2_mpm_mls
#     LIBS glfw
#     FEATURES
#         UTILS_TASK_GRAPH
#         UTILS_IMGUI
#         UTILS_PIPELINE_MANAGER_GLSLANG
#         UTILS_PIPELINE_MANAGER_SLANG
# )
DAXA_CREATE_TEST(
    FOLDER 3_samples 3_hello_triangle_compute
    LIBS glfw
    FEATURES
        UTILS_IMGUI
        UTILS_PIPELINE_MANAGER_GLSLANG
)

DAXA_CREATE_TEST(
    FOLDER 3_samples 5_boids
    LIBS glfw
    FEATURES
        UTILS_TASK_GRAPH
        UTILS_PIPELINE_MANAGER_GLSLANG
)

DAXA_CREATE_TEST(
    FOLDER 4_hello_daxa 1_pink_screen
    LIBS glfw
)

DAXA_CREATE_TEST(
    FOLDER 4_hello_daxa 2_triangle
    LIBS glfw
    FEATURES
        UTILS_TASK_GRAPH
        UTILS_PIPELINE_MANAGER_GLSLANG
)

DAXA_CREATE_TEST_C(
    FOLDER 4_hello_daxa 0_c_api
    LIBS glfw Vulkan::Vulkan
)
