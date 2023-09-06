
if(NOT (CMAKE_TOOLCHAIN_FILE MATCHES "/scripts/buildsystems/vcpkg.cmake") AND DEFINED CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE UNINITIALIZED "")
endif()

if(EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOT)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
else()
    find_package(Git REQUIRED)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
        execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/Microsoft/vcpkg
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND_ERROR_IS_FATAL ANY)
    else()
        execute_process(COMMAND ${GIT_EXECUTABLE} pull
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vcpkg"
            COMMAND_ERROR_IS_FATAL ANY)
    endif()
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()
