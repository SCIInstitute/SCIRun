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

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# Common CMake args
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON

  # Force output directories so we can skip the install step
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=<INSTALL_DIR>/bin

  # Multi-config (VS) subdirs
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/bin
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/bin
)

# For single-config generators (Ninja, Makefiles), propagate build type
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# Where to build/install inside the superbuild tree
set(_tetgen_src "${CMAKE_BINARY_DIR}/Externals/Source/Tetgen_external")
set(_tetgen_bin "${CMAKE_BINARY_DIR}/Externals/Build/Tetgen_external")
set(_tetgen_inst "${CMAKE_BINARY_DIR}/Externals/Install/Tetgen_external")

ExternalProject_Add(Tetgen_external
  GIT_REPOSITORY          https://github.com/CIBC-Internal/TetGen.git
  GIT_TAG                 v1.6.1            # or your tag like v1.6.0-scirun1
  UPDATE_DISCONNECTED     1                 # speed up: don't ping remote every run

  SOURCE_DIR              ${_tetgen_src}
  BINARY_DIR              ${_tetgen_bin}

  CMAKE_GENERATOR         "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS              ${_cmake_args}

  # We direct outputs to <INSTALL_DIR>, so a separate "install" step is unnecessary.
  INSTALL_COMMAND         ""

  LOG_CONFIGURE           1
  LOG_BUILD               1
  LOG_INSTALL             1
)

ExternalProject_Get_Property(Tetgen_external SOURCE_DIR)
ExternalProject_Get_Property(Tetgen_external BINARY_DIR)
ExternalProject_Get_Property(Tetgen_external INSTALL_DIR)
SET(TETGEN_INCLUDE ${SOURCE_DIR})
SET(TETGEN_LIBRARY_DIR ${BINARY_DIR})
SET(TETGEN_USE_FILE ${INSTALL_DIR}/UseTetgen.cmake)
# see Tetgen CMakeLists.txt file
SET(TETGEN_LIBRARY "tet")
SET(Tetgen_DIR ${INSTALL_DIR} CACHE PATH "")

# Boost is special case - normally this should be handled in external library repo
CONFIGURE_FILE(${SUPERBUILD_DIR}/TetgenConfig.cmake.in ${INSTALL_DIR}/TetgenConfig.cmake @ONLY)
CONFIGURE_FILE(${SUPERBUILD_DIR}/UseTetgen.cmake ${TETGEN_USE_FILE} COPYONLY)

MESSAGE(STATUS "Tetgen_DIR: ${Tetgen_DIR}")
