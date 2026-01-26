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

# TetgenExternal.cmake
set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# Common CMake args
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

  # Force output directories so install() is not needed
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=<INSTALL_DIR>/bin

  # Multi-config versions for VS
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/bin
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/bin
)

# Only add CMAKE_BUILD_TYPE for single-config generators (e.g. Ninja)
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

ExternalProject_Add(Tetgen_external
  URL "https://github.com/CIBC-Internal/SCIRunTestData/releases/download/test/tetgen1.5.1-beta1.tar.gz"
  PATCH_COMMAND ""

  CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS ${_cmake_args}

  # Skip install entirely
  INSTALL_COMMAND ""

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

ExternalProject_Get_Property(Tetgen_external SOURCE_DIR INSTALL_DIR)
message(STATUS "[Tetgen_external] INSTALL_DIR=${INSTALL_DIR}")

# Tetgen headers live in the source tree
set(TETGEN_INCLUDE     ${SOURCE_DIR})
set(TETGEN_LIBRARY_DIR ${INSTALL_DIR}/lib)
set(TETGEN_LIBRARY     "tet")   # static or shared, same name root

set(TETGEN_USE_FILE    ${INSTALL_DIR}/UseTetgen.cmake)
set(Tetgen_DIR ${INSTALL_DIR} CACHE PATH "")

# If you have these files, you can still generate them:
configure_file(${SUPERBUILD_DIR}/TetgenConfig.cmake.in
               ${INSTALL_DIR}/TetgenConfig.cmake @ONLY)

configure_file(${SUPERBUILD_DIR}/UseTetgen.cmake
               ${TETGEN_USE_FILE} COPYONLY)