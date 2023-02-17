vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/glfw/glfw
    REF 8f470597d625ae28758c16b4293dd42d63e8a83a
)

if(VCPKG_TARGET_IS_LINUX)
    message(
"GLFW3 currently requires the following libraries from the system package manager:
    xinerama
    xcursor
    xorg
    libglu1-mesa
These can be installed on Ubuntu systems via sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev")
endif()

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    xlib WITH_XLIB
    wayland WITH_WAYLAND
)
set(GLFW_DEFINES)
if(WITH_XLIB)
    list(APPEND GLFW_DEFINES "-DGLFW_BUILD_X11=true")
endif()
if(WITH_WAYLAND)
    list(APPEND GLFW_DEFINES "-DGLFW_BUILD_WAYLAND=true")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_DOCS=OFF
        ${GLFW_DEFINES}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/glfw3)
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE.md" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

vcpkg_copy_pdbs()
