#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.

# TnyExternal.cmake (updated)

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# Final install prefix for Tny artifacts
set(_tny_inst "${CMAKE_BINARY_DIR}/Externals/Install/Tny_external")

# 1) Fetch upstream Tny (no configure/build/install here)
ExternalProject_Add(Tny_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Tny.git"
  GIT_TAG        "origin/master"
  UPDATE_DISCONNECTED 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  LOG_DOWNLOAD 1
  LOG_UPDATE   1
)

ExternalProject_Get_Property(Tny_external SOURCE_DIR)

# 2) Copy headers (your existing step)
#    Place them under <install>/include/tny/ so #include <tny/tny.h> works
ExternalProject_Add_Step(Tny_external copy_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_tny_inst}/include/tny
  COMMAND ${CMAKE_COMMAND} -Dsrc=${SOURCE_DIR}/src/tny -Ddst=${_tny_inst}/include/tny
          -P ${CMAKE_CURRENT_LIST_DIR}/TnyInstall.cmake
  DEPENDEES download
  DEPENDERS build
  COMMENT "[Tny_external] Copying headers"
)

# 3) Build a small static library from tny.c using CMake -E cmake_script
#    We create a build.ninja/msbuild generator-less compile via a one-off CMake configure+build
set(_tny_build_dir "${CMAKE_BINARY_DIR}/Externals/Build/Tny_wrapper")
file(MAKE_DIRECTORY "${_tny_build_dir}")

# Generate a minimal CMakeLists.txt for building the library
file(WRITE "${_tny_build_dir}/CMakeLists.txt" "
cmake_minimum_required(VERSION 3.16)
project(tny_wrapper C)
add_library(tny STATIC \"${SOURCE_DIR}/src/tny/tny.c\")
target_include_directories(tny PUBLIC \"${_tny_inst}/include\")
if(MSVC)
  set_target_properties(tny PROPERTIES DEBUG_POSTFIX d)
endif()
install(TARGETS tny
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
")

# Configure & build the wrapper, then install to _tny_inst
ExternalProject_Add(Tny_wrapper_external
  SOURCE_DIR "${_tny_build_dir}"
  BINARY_DIR "${_tny_build_dir}/_build"
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${_tny_inst}
    -DCMAKE_BUILD_TYPE:STRING=$<IF:$<CONFIG:Debug>,Debug,Release>
  BUILD_ALWAYS 1
  INSTALL_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target install
  DEPENDS Tny_external
  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# 4) Install a proper TnyConfig.cmake for find_package(Tny CONFIG)
add_custom_target(Tny_export ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_tny_inst}/lib/cmake/Tny
  COMMAND ${CMAKE_COMMAND}
          -DTNY_INC:PATH=${_tny_inst}/include
          -DTNY_LIB:PATH=${_tny_inst}/lib/$<IF:$<CONFIG:Debug>,tnyd.lib,tny.lib>
          -P ${CMAKE_CURRENT_LIST_DIR}/TnyWriteConfig.cmake
  DEPENDS Tny_wrapper_external
  COMMENT "[Tny_external] Writing TnyConfig.cmake"
)

# Helper script to expand the @ONLY template with the correct lib for the current config
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/TnyWriteConfig.cmake" "
set(_cfg_dir \"${_tny_inst}/lib/cmake/Tny\")
file(MAKE_DIRECTORY \"\${_cfg_dir}\")
configure_file(\"${SUPERBUILD_DIR}/TnyConfig.cmake.in\" \"\${_cfg_dir}/TnyConfig.cmake\" @ONLY)
")

# Export paths for downstream (SCIRun)
set(TNY_INSTALL_DIR "${_tny_inst}")
set(TNY_INCLUDE     "${TNY_INSTALL_DIR}/include")
set(TNY_LIBRARY_DIR "${TNY_INSTALL_DIR}/lib")
message(STATUS "[Tny_external] INSTALL_DIR=${TNY_INSTALL_DIR}")