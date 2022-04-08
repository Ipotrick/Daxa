# -------------

# Common Ambient Variables:
#   CURRENT_BUILDTREES_DIR    = ${VCPKG_ROOT_DIR}\buildtrees\${PORT}
#   CURRENT_PACKAGES_DIR      = ${VCPKG_ROOT_DIR}\packages\${PORT}_${TARGET_TRIPLET}
#   CURRENT_PORT_DIR          = ${VCPKG_ROOT_DIR}\ports\${PORT}
#   CURRENT_INSTALLED_DIR     = ${VCPKG_ROOT_DIR}\installed\${TRIPLET}
#   DOWNLOADS                 = ${VCPKG_ROOT_DIR}\downloads
#   PORT                      = current port name (zlib, etc)
#   TARGET_TRIPLET            = current triplet (x86-windows, x64-windows-static, etc)
#   VCPKG_CRT_LINKAGE         = C runtime linkage type (static, dynamic)
#   VCPKG_LIBRARY_LINKAGE     = target library linkage type (static, dynamic)
#   VCPKG_ROOT_DIR            = <C:\path\to\current\vcpkg>
#   VCPKG_TARGET_ARCHITECTURE = target architecture (x64, x86, arm)
#   VCPKG_TOOLCHAIN           = ON OFF
#   TRIPLET_SYSTEM_ARCH       = arm x86 x64
#   BUILD_ARCH                = "Win32" "x64" "ARM"
#   MSBUILD_PLATFORM          = "Win32"/"x64"/${TRIPLET_SYSTEM_ARCH}
#   DEBUG_CONFIG              = "Debug Static" "Debug Dll"
#   RELEASE_CONFIG            = "Release Static"" "Release DLL"
#   VCPKG_TARGET_IS_WINDOWS
#   VCPKG_TARGET_IS_UWP
#   VCPKG_TARGET_IS_LINUX
#   VCPKG_TARGET_IS_OSX
#   VCPKG_TARGET_IS_FREEBSD
#   VCPKG_TARGET_IS_ANDROID
#   VCPKG_TARGET_IS_MINGW
#   VCPKG_TARGET_EXECUTABLE_SUFFIX
#   VCPKG_TARGET_STATIC_LIBRARY_SUFFIX
#   VCPKG_TARGET_SHARED_LIBRARY_SUFFIX
#
# 	See additional helpful variables in /docs/maintainers/vcpkg_common_definitions.md

# # Specifies if the port install should fail immediately given a condition
# vcpkg_fail_port_install(MESSAGE "dxc currently only supports Linux and Mac platforms" ON_TARGET "Windows")

vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.6.2112/dxc_2021_12_08.zip"
    FILENAME "dxc_2021_12_08.zip"
    SHA512 e9b36e896c1d47b39b648adbecf44da7f8543216fd1df539760f0c591907aea081ea6bfc59eb927073aaa1451110c5dc63003546509ff84c9e4445488df97c27
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
    NO_REMOVE_ONE_LEVEL
    # (Optional) A friendly name to use instead of the filename of the archive (e.g.: a version number or tag).
    # REF 1.0.0
    # (Optional) Read the docs for how to generate patches at:
    # https://github.com/Microsoft/vcpkg/blob/master/docs/examples/patching.md
    # PATCHES
    #   001_port_fixes.patch
    #   002_more_port_fixes.patch
)

# # Check if one or more features are a part of a package installation.
# # See /docs/maintainers/vcpkg_check_features.md for more details
# vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
#   FEATURES # <- Keyword FEATURES is required because INVERTED_FEATURES are being used
#     tbb   WITH_TBB
#   INVERTED_FEATURES
#     tbb   ROCKSDB_IGNORE_PACKAGE_TBB
# )
file(WRITE ${SOURCE_PATH}/CMakeLists.txt [==[
cmake_minimum_required(VERSION 3.21)
project(dxc VERSION 0.1.0)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dxc-config.cmake [=[
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

add_library(dxc::dxc SHARED IMPORTED)
set_target_properties(dxc::dxc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
    INTERFACE_LINK_DIRECTORIES "${_IMPORT_PREFIX}/lib"
    
    IMPORTED_LOCATION ${_IMPORT_PREFIX}/bin/dxcompiler.dll
    IMPORTED_IMPLIB   ${_IMPORT_PREFIX}/lib/dxcompiler.lib
)
]=])

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/dxc-config.cmake
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/dxc)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/inc/ TYPE INCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/x64/ TYPE BIN PATTERN "*.exe" EXCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib/x64/ TYPE LIB)

]==])

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA # Disable this option if project cannot be built with Ninja
    # OPTIONS -DUSE_THIS_IN_ALL_BUILDS=1 -DUSE_THIS_TOO=2
    # OPTIONS_RELEASE -DOPTIMIZE=1
    # OPTIONS_DEBUG -DDEBUGGABLE=1
)

vcpkg_install_cmake()

# # Moves all .cmake files from /debug/share/dxc/ to /share/dxc/
# # See /docs/maintainers/vcpkg_fixup_cmake_targets.md for more details
# vcpkg_fixup_cmake_targets(CONFIG_PATH cmake TARGET_PATH share/dxc)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Handle copyright (copied verbatim from github.com/microsoft/DirectXShaderCompiler)
file(WRITE ${CURRENT_PACKAGES_DIR}/share/dxc/copyright [=[
==============================================================================
LLVM Release License
==============================================================================
University of Illinois/NCSA
Open Source License

Copyright (c) 2003-2015 University of Illinois at Urbana-Champaign.
All rights reserved.

Developed by:

    LLVM Team

    University of Illinois at Urbana-Champaign

    http://llvm.org

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal with
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimers.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimers in the
      documentation and/or other materials provided with the distribution.

    * Neither the names of the LLVM Team, University of Illinois at
      Urbana-Champaign, nor the names of its contributors may be used to
      endorse or promote products derived from this Software without specific
      prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
SOFTWARE.

==============================================================================
Copyrights and Licenses for Third Party Software Distributed with LLVM:
==============================================================================
The LLVM software contains code written by third parties.  Such software will
have its own individual LICENSE.TXT file in the directory in which it appears.
This file will describe the copyrights, license, and restrictions which apply
to that code.

The disclaimer of warranty in the University of Illinois Open Source License
applies to all code in the LLVM Distribution, and nothing in any of the
other licenses gives permission to use the names of the LLVM Team or the
University of Illinois to endorse or promote products derived from this
Software.

The following pieces of software have additional or alternate copyrights,
licenses, and/or restrictions:

Program             Directory
-------             ---------
Autoconf            llvm/autoconf
                    llvm/projects/ModuleMaker/autoconf
Google Test         llvm/utils/unittest/googletest
OpenBSD regex       llvm/lib/Support/{reg*, COPYRIGHT.regex}
pyyaml tests        llvm/test/YAMLParser/{*.data, LICENSE.TXT}
ARM contributions   llvm/lib/Target/ARM/LICENSE.TXT
md5 contributions   llvm/lib/Support/MD5.cpp llvm/include/llvm/Support/MD5.h
miniz               llvm/lib/Miniz/miniz.c llvm/include/miniz/miniz.h llvm/lib/Miniz/LICENSE.txt
]=]
)
