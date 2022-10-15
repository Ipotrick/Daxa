# temporary fix for wayland

if(NOT WIN32)
    if(Wayland_INCLUDE_DIR AND Wayland_LIBRARIES)
        # In the cache already
        set(Wayland_FIND_QUIETLY TRUE)
    endif()

    # Use pkg-config to get the directories and then use these values
    # in the find_path() and find_library() calls
    find_package(PkgConfig)
    pkg_check_modules(PKG_Wayland QUIET wayland-client wayland-server wayland-egl wayland-cursor)

    set(Wayland_DEFINITIONS ${PKG_Wayland_CFLAGS})

    find_path(Wayland_CLIENT_INCLUDE_DIR  NAMES wayland-client.h HINTS ${PKG_Wayland_INCLUDE_DIRS})
    find_path(Wayland_SERVER_INCLUDE_DIR  NAMES wayland-server.h HINTS ${PKG_Wayland_INCLUDE_DIRS})
    find_path(Wayland_EGL_INCLUDE_DIR     NAMES wayland-egl.h    HINTS ${PKG_Wayland_INCLUDE_DIRS})
    find_path(Wayland_CURSOR_INCLUDE_DIR  NAMES wayland-cursor.h HINTS ${PKG_Wayland_INCLUDE_DIRS})

    find_library(Wayland_CLIENT_LIBRARIES NAMES wayland-client   HINTS ${PKG_Wayland_LIBRARY_DIRS})
    find_library(Wayland_SERVER_LIBRARIES NAMES wayland-server   HINTS ${PKG_Wayland_LIBRARY_DIRS})
    find_library(Wayland_EGL_LIBRARIES    NAMES wayland-egl      HINTS ${PKG_Wayland_LIBRARY_DIRS})
    find_library(Wayland_CURSOR_LIBRARIES NAMES wayland-cursor   HINTS ${PKG_Wayland_LIBRARY_DIRS})

    set(Wayland_INCLUDE_DIR ${Wayland_CLIENT_INCLUDE_DIR} ${Wayland_SERVER_INCLUDE_DIR} ${Wayland_EGL_INCLUDE_DIR} ${Wayland_CURSOR_INCLUDE_DIR})
    set(Wayland_LIBRARIES ${Wayland_CLIENT_LIBRARIES} ${Wayland_SERVER_LIBRARIES} ${Wayland_EGL_LIBRARIES} ${Wayland_CURSOR_LIBRARIES})
    list(REMOVE_DUPLICATES Wayland_INCLUDE_DIR)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Wayland DEFAULT_MSG Wayland_LIBRARIES Wayland_INCLUDE_DIR)

    mark_as_advanced(Wayland_INCLUDE_DIR Wayland_LIBRARIES)
endif()
