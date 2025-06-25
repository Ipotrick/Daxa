cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/warnings.cmake")
if(DAXA_USE_VCPKG)
    include("${CMAKE_CURRENT_LIST_DIR}/cmake/vcpkg.cmake")
endif()

if(DAXA_ENABLE_UTILS_TASK_GRAPH)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-task-graph")
    set(DAXA_ENABLE_UTILS_MEM true)
endif()
if(DAXA_ENABLE_UTILS_FSR2)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-fsr2")
endif()
if(DAXA_ENABLE_UTILS_IMGUI)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-imgui")
endif()
if(DAXA_ENABLE_UTILS_MEM)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-mem")
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-pipeline-manager-glslang")
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SLANG)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-pipeline-manager-slang")
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-pipeline-manager-spirv-validation")
endif()
if(DAXA_ENABLE_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()
if(DAXA_USE_STREAMLINE)
    list(APPEND VCPKG_MANIFEST_FEATURES "utils-streamline")
endif()

project(daxa VERSION 3.0.2)

if(DAXA_ENABLE_STATIC_ANALYSIS)
    set(CPPCHECK_TEMPLATE "gcc")
    find_program(CPPCHECK cppcheck)
    find_program(CLANG_TIDY clang-tidy)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK}
            --template=${CPPCHECK_TEMPLATE}
            --enable=style,performance,warning,portability
            --inline-suppr
            --suppress=cppcheckError
            --suppress=internalAstError
            --suppress=unmatchedSuppression
            --suppress=preprocessorErrorDirective
            --suppress=exceptThrowInDestructor
            --suppress=functionStatic
            --inconclusive)
    endif()
    if(CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY}
            --fix)
    endif()
else()
    # TODO(capi): re-enable when not dealing with the compile errors.
    # Also, the features in the CMakePresets are disabled for now
    set(CMAKE_UNITY_BUILD true)
endif()

add_library(daxa
    "src/cpp_wrapper.cpp"

    "src/impl_device.cpp"
    "src/impl_features.cpp"
    "src/impl_instance.cpp"
    "src/impl_core.cpp"
    "src/impl_pipeline.cpp"
    "src/impl_swapchain.cpp"
    "src/impl_command_recorder.cpp"
    "src/impl_gpu_resources.cpp"
    "src/impl_sync.cpp"
    "src/impl_dependencies.cpp"
    "src/impl_timeline_query.cpp"

    "src/utils/impl_task_graph.cpp"
    "src/utils/impl_task_graph_mk2.cpp"
    "src/utils/impl_imgui.cpp"
    "src/utils/impl_fsr2.cpp"
    "src/utils/impl_mem.cpp"
    "src/utils/impl_pipeline_manager.cpp"
    "src/utils/impl_streamline.cpp"
)

add_library(daxa::daxa ALIAS daxa)

if(BUILD_SHARED_LIBS)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(${PROJECT_NAME}
            PRIVATE "$<IF:$<CXX_COMPILER_ID:Clang>,DAXA_CMAKE_EXPORT=[[gnu::dllexport]],DAXA_CMAKE_EXPORT=__declspec(dllexport)>"
            INTERFACE "$<IF:$<CXX_COMPILER_ID:Clang>,DAXA_CMAKE_EXPORT=[[gnu::dllimport]],DAXA_CMAKE_EXPORT=__declspec(dllimport)>")
    else()
        target_compile_definitions(${PROJECT_NAME}
            PUBLIC "DAXA_CMAKE_EXPORT=")
    endif()
else()
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC "DAXA_CMAKE_EXPORT=")
endif()

target_compile_features(daxa PUBLIC cxx_std_20)

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

include(GNUInstallDirs)

target_compile_definitions(daxa
    PUBLIC
    $<BUILD_INTERFACE:DAXA_SHADER_INCLUDE_DIR="${CMAKE_CURRENT_LIST_DIR}/include">
)

if(DAXA_USE_STATIC_CRT AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(DAXA_ENABLE_UTILS_FSR2)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_FSR2=true
    )
    find_package(fsr2 CONFIG REQUIRED)
    target_link_libraries(daxa
        PRIVATE
        fsr2::ffx_fsr2_api
        fsr2::ffx_fsr2_api_vk
    )
endif()
if(DAXA_ENABLE_UTILS_IMGUI)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_IMGUI=true
    )
    find_package(imgui CONFIG REQUIRED)
    target_link_libraries(daxa
        PUBLIC
        imgui::imgui
    )
endif()
if(DAXA_ENABLE_UTILS_MEM)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_MEM=true
    )
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG=true
    )
    find_package(glslang CONFIG REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(daxa
        PRIVATE
        glslang::glslang
        glslang::SPIRV
    )
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SLANG)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG=true
    )
    find_library(SLANG_LIB slang)
    target_link_libraries(daxa PRIVATE ${SLANG_LIB})

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # NOTE(grundlett): Really hacky. It appears there's no way to create plugin dll dependencies...
        # see https://gitlab.kitware.com/cmake/cmake/-/issues/22993
        find_program(SLANG_DLL slang.dll)
        find_program(SLANG_GLSLANG_DLL slang-glslang.dll)
        add_custom_command(TARGET daxa POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${SLANG_DLL} $<TARGET_FILE_DIR:daxa>
            COMMAND ${CMAKE_COMMAND} -E copy ${SLANG_GLSLANG_DLL} $<TARGET_FILE_DIR:daxa>
        )
    else()
        # TODO(grundlett) ...
        # find_file(SLANG_DLL libslang.so)
        # find_file(SLANG_GLSLANG_DLL libslang-glslang.so)
    endif()
endif()
if(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION=true
    )
    find_package(SPIRV-Tools CONFIG REQUIRED)
    target_link_libraries(daxa
        PRIVATE
        SPIRV-Tools-static
    )
endif()
if(DAXA_ENABLE_UTILS_TASK_GRAPH)
    target_compile_definitions(daxa
        PUBLIC
        DAXA_BUILT_WITH_UTILS_TASK_GRAPH=true
    )
endif()

target_link_libraries(daxa
    PRIVATE
    GPUOpen::VulkanMemoryAllocator
)

target_include_directories(daxa
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11)
    if(X11_FOUND)
        target_compile_definitions(daxa
            PUBLIC
            DAXA_BUILT_WITH_X11=true
        )
        target_link_libraries(daxa
            PRIVATE
            ${X11_LIBRARIES}
        )
    endif()

    find_package(Wayland)
    if(Wayland_FOUND)
        target_compile_definitions(daxa
            PUBLIC
            DAXA_BUILT_WITH_WAYLAND=true
        )
        target_link_libraries(daxa
            PRIVATE
            ${Wayland_LIBRARIES}
        )
    endif()
endif()

set_project_warnings(daxa)


# === STREAMLINE SMART INTEGRATION ===

option(DAXA_USE_STREAMLINE "Use Streamline to access Nvidia DLSS+ tools" OFF)
set(STREAMLINE_ENABLED FALSE)
set(STREAMLINE_MODE "NONE")
set(STREAMLINE_ENABLED_FOR_FRAME TRUE)

option(DAXA_USE_DYNAMIC_VULKAN "Use dynamic loading for Vulkan functions" OFF)

if(DAXA_USE_STREAMLINE)
    set(STREAMLINE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/Streamline-sdk")
    if(EXISTS "${STREAMLINE_SDK_DIR}/include/sl.h")
        message(STATUS "✓ Found official Streamline SDK")
        set(STREAMLINE_ENABLED TRUE)
        set(STREAMLINE_MODE "SDK")
        
        # Use expression generators to differentiate build/install
        target_include_directories(${PROJECT_NAME} 
            PUBLIC 
            $<BUILD_INTERFACE:${STREAMLINE_SDK_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/streamline>
        )
        
        if(WIN32 AND EXISTS "${STREAMLINE_SDK_DIR}/lib/x64/sl.interposer.lib")
            target_link_libraries(${PROJECT_NAME} 
                PRIVATE 
                $<BUILD_INTERFACE:${STREAMLINE_SDK_DIR}/lib/x64/sl.interposer.lib>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}/sl.interposer.lib>
            )
            message(STATUS "✓ Linked Streamline library")
            
            # Copy DLLs
            file(GLOB STREAMLINE_DLLS "${STREAMLINE_SDK_DIR}/bin/x64/*.dll")
            if(STREAMLINE_DLLS)
                list(LENGTH STREAMLINE_DLLS DLL_COUNT)
                message(STATUS "✓ Found ${DLL_COUNT} Streamline DLLs")
                foreach(DLL_PATH ${STREAMLINE_DLLS})
                    get_filename_component(DLL_NAME ${DLL_PATH} NAME)
                    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${DLL_PATH}"
                            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                        COMMENT "Copying ${DLL_NAME}"
                    )
                endforeach()
            endif()
        endif()
        
        # Install Streamline files when DAXA_INSTALL is active
        if(DAXA_INSTALL)
            install(DIRECTORY "${STREAMLINE_SDK_DIR}/include/"
                DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/streamline"
                FILES_MATCHING PATTERN "*.h"
            )
            
            if(WIN32 AND EXISTS "${STREAMLINE_SDK_DIR}/lib/x64/sl.interposer.lib")
                install(FILES "${STREAMLINE_SDK_DIR}/lib/x64/sl.interposer.lib"
                    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                )
            endif()
            
            if(STREAMLINE_DLLS)
                install(FILES ${STREAMLINE_DLLS}
                    DESTINATION "${CMAKE_INSTALL_BINDIR}"
                )
            endif()
        endif()
    endif()
endif()

# Macro configuration based on mode
if(STREAMLINE_ENABLED AND DAXA_USE_STREAMLINE)
    set(DAXA_USE_DYNAMIC_VULKAN TRUE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SL_VULKAN=1
    )
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        STREAMLINE_ENABLED=1
    )
    if(STREAMLINE_ENABLED_FOR_FRAME) 
        target_compile_definitions(${PROJECT_NAME} PUBLIC
            STREAMLINE_ENABLED_FOR_FRAME=1
        )
    endif()

    # Specific mode
    if(STREAMLINE_MODE STREQUAL "SDK")
        target_compile_definitions(${PROJECT_NAME} PRIVATE STREAMLINE_SDK_MODE=1)
    elseif(STREAMLINE_MODE STREQUAL "COMPILED")
        target_compile_definitions(${PROJECT_NAME} PRIVATE STREAMLINE_COMPILED_MODE=1)
    elseif(STREAMLINE_MODE STREQUAL "HEADERS_ONLY")
        target_compile_definitions(${PROJECT_NAME} PRIVATE STREAMLINE_HEADERS_ONLY=1)
    endif()
    
    message(STATUS "🚀 Streamline integration: ${STREAMLINE_MODE}")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE STREAMLINE_DISABLED=1)
    message(STATUS "❌ Streamline integration: DISABLED")
endif()

if(DAXA_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

if(DAXA_ENABLE_TOOLS)
    add_executable(daxa_tools_compile_imgui_shaders "src/utils/impl_imgui.cpp")
    target_compile_definitions(daxa_tools_compile_imgui_shaders PRIVATE DAXA_COMPILE_IMGUI_SHADERS=true)
    target_link_libraries(daxa_tools_compile_imgui_shaders PRIVATE daxa::daxa)
endif()

if(DAXA_INSTALL)
    include("${CMAKE_CURRENT_LIST_DIR}/cmake/packaging.cmake")
endif()

if(DAXA_USE_DYNAMIC_VULKAN)
    message(STATUS "✓ Vulkan functions will be loaded dynamically")
    add_definitions(-DVK_NO_PROTOTYPES)
    add_definitions(-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1)
    add_definitions(-DVMA_STATIC_VULKAN_FUNCTIONS=0)
    add_definitions(-DDAXA_USE_DYNAMIC_VULKAN=1)
else()
    message(STATUS "✓ Vulkan functions will be loaded statically")
    target_link_libraries(daxa PRIVATE Vulkan::Vulkan)
endif()

target_include_directories(daxa
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11)
    if(X11_FOUND)
        target_compile_definitions(daxa
            PUBLIC
            DAXA_BUILT_WITH_X11=true
        )
        target_link_libraries(daxa
            PRIVATE
            ${X11_LIBRARIES}
        )
    endif()

    find_package(Wayland)
    if(Wayland_FOUND)
        target_compile_definitions(daxa
            PUBLIC
            DAXA_BUILT_WITH_WAYLAND=true
        )
        target_link_libraries(daxa
            PRIVATE
            ${Wayland_LIBRARIES}
        )
    endif()
endif()

set_project_warnings(daxa)