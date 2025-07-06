find_package(Vulkan REQUIRED)

include(FetchContent)

FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG        v3.1.0
    SYSTEM
)
FetchContent_MakeAvailable(vma)

if (DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG AND NOT TARGET glslang::glslang)
    option(ENABLE_OPT "" OFF)
    set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
    set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
    set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
    set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG        5398d55e33dff7d26fecdd2c35808add986c558c
        #SYSTEM
    )
    FetchContent_MakeAvailable(glslang)
endif()

if (DAXA_ENABLE_TESTS AND NOT TARGET glfw)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG        3.4
    )
    FetchContent_MakeAvailable(glfw)
endif()

if (DAXA_ENABLE_UTILS_IMGUI AND NOT TARGET imgui::imgui)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG        fdc084f532189fda8474079f79e74fa5e3541c9f
    )

    FetchContent_GetProperties(imgui)
    if(NOT imgui_POPULATED)
        FetchContent_MakeAvailable(imgui)

        add_library(lib_imgui
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)

        target_include_directories(lib_imgui PUBLIC
            ${imgui_SOURCE_DIR} 
            ${imgui_SOURCE_DIR}/backends
            ${Vulkan_INCLUDE_DIRS}
            ${glfw_SOURCE_DIR}/include)

        target_link_libraries(lib_imgui PRIVATE glfw)
        add_library(imgui::imgui ALIAS lib_imgui)
    endif()
endif()

if (DAXA_ENABLE_UTILS_PIPELINE_MANAGER_SLANG AND NOT TARGET slang::slang)
    set(Slang_VERSION "2025.11")
    FetchContent_Declare(
        slang
        URL https://github.com/shader-slang/slang/releases/download/v${Slang_VERSION}/slang-${Slang_VERSION}-windows-x86_64.zip
        # URL https://github.com/shader-slang/slang/releases/download/v2025.11/slang-2025.11-windows-x86_64.zip
    )
    FetchContent_MakeAvailable(slang)

    find_path(Slang_INCLUDE_DIR
        slang.h
        HINTS ${slang_SOURCE_DIR}/include
        NO_DEFAULT_PATH
        DOC "Directory that includes slang.h."
    )
    mark_as_advanced(Slang_INCLUDE_DIR)

    find_library(Slang_LIBRARY
        NAMES slang
        HINTS ${slang_SOURCE_DIR}/lib
        NO_DEFAULT_PATH
        DOC "Slang linker library"
    )
    mark_as_advanced(Slang_LIBRARY)

    if(WIN32)
        find_file(Slang_DLL
            NAMES slang.dll
            HINTS ${slang_SOURCE_DIR}/bin
            NO_DEFAULT_PATH
            DOC "Slang shared library (.dll)"
        )
    else() # Unix; uses .so
        set(Slang_DLL ${Slang_LIBRARY} CACHE PATH "Slang shared library (.so)")
    endif()
    mark_as_advanced(Slang_DLL)

    add_library(Slang SHARED IMPORTED)

    set_target_properties(
        Slang PROPERTIES
        IMPORTED_LOCATION ${Slang_DLL}
        # NOTE(nbickford): Setting INTERFACE_INCLUDE_DIRECTORIES
        # should make the include directory propagate upwards...
        # but in CMake 3.31.6, it doesn't. In fact, it does the
        # opposite; adding INTERFACE_INCLUDE_DIRECTORIES makes
        # attempts to add it later have no effect.
        # INTERFACE_INCLUDE_DIRECTORIES ${Slang_INCLUDE_DIR}
    )
    if(WIN32)
        set_property(TARGET Slang PROPERTY IMPORTED_IMPLIB ${Slang_LIBRARY})
    else()
        # Vulkan SDK includes 'libslang.so' and sets LD_LIBRARY_PATH, which conflict
        # with the downloaded slang. This uses the deprecated RPATH instead of
        # RUNPATH to take priority over LD_LIBRARY_PATH.
        set_target_properties(Slang PROPERTIES
            INTERFACE_LINK_OPTIONS "-Wl,--disable-new-dtags"
        )
    endif()

    # Additionally, SLANG_OPTIMIZATION_LEVEL_HIGH requires slang-glslang.dll.
    # Find it and link with it by default:
    find_file(Slang_GLSLANG
        NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}slang-glslang${CMAKE_SHARED_LIBRARY_SUFFIX}
        HINTS ${slang_SOURCE_DIR}/bin
                ${slang_SOURCE_DIR}/lib
        NO_DEFAULT_PATH
        DOC "slang-glslang shared library"
    )
    mark_as_advanced(Slang_GLSLANG)

    add_library(SlangGlslang SHARED IMPORTED)
    set_property(TARGET SlangGlslang PROPERTY IMPORTED_LOCATION ${Slang_GLSLANG})
    if(WIN32)
        set_property(TARGET SlangGlslang PROPERTY IMPORTED_IMPLIB ${Slang_LIBRARY})
    endif()

    add_library(slang::slang ALIAS Slang)
    add_library(slang::glslang ALIAS SlangGlslang)

    set_property(TARGET Slang PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Slang_INCLUDE_DIR})
endif()

if (DAXA_ENABLE_UTILS_FSR3)
    FetchContent_Declare(
        ffx_sdk
        URL https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/releases/download/v1.1.4/FidelityFX-SDK-v1.1.4.zip
        SOURCE_SUBDIR "nonexistent-subdir"
    )
    FetchContent_MakeAvailable(ffx_sdk)

    find_path(ffx_sdk_INCLUDE_DIR
        ffx_api/ffx_api.h
        HINTS ${ffx_sdk_SOURCE_DIR}/ffx-api/include
        NO_DEFAULT_PATH
        DOC "Directory that includes ffx_api/ffx_api.h."
    )
    mark_as_advanced(ffx_sdk_INCLUDE_DIR)

    find_library(ffx_sdk_LIBRARY
        NAMES amd_fidelityfx_vk
        HINTS ${ffx_sdk_SOURCE_DIR}/ffx-api/bin
        NO_DEFAULT_PATH
        DOC "ffx_sdk linker library"
    )
    mark_as_advanced(ffx_sdk_LIBRARY)

    find_file(ffx_sdk_DLL
        NAMES amd_fidelityfx_vk.dll
        HINTS ${ffx_sdk_SOURCE_DIR}/ffx-api/bin
        NO_DEFAULT_PATH
        DOC "ffx_sdk shared library (.dll)"
    )
    mark_as_advanced(ffx_sdk_DLL)

    add_library(lib_ffx_sdk SHARED IMPORTED)

    set_target_properties(
        lib_ffx_sdk PROPERTIES
        IMPORTED_LOCATION ${ffx_sdk_DLL}
    )
    set_property(TARGET lib_ffx_sdk PROPERTY IMPORTED_IMPLIB ${ffx_sdk_LIBRARY})

    add_library(ffx_sdk::ffx_sdk ALIAS lib_ffx_sdk)

    set_property(TARGET lib_ffx_sdk PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${ffx_sdk_INCLUDE_DIR})
endif()
