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

# -------------------------------------------------------
# TetgenExternal.cmake - unified Windows/macOS/Linux version
# -------------------------------------------------------

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# Common CMake args for external TetGen
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON

  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=<INSTALL_DIR>/bin
)

# Handle single‑config (Ninja/Makefiles)
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

if(CMAKE_OSX_DEPLOYMENT_TARGET)
  list(APPEND _cmake_args -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

# Superbuild paths
set(_tetgen_src  "${CMAKE_BINARY_DIR}/Externals/Source/Tetgen_external")
set(_tetgen_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Tetgen_external")
set(_tetgen_inst "${CMAKE_BINARY_DIR}/Externals/Install/Tetgen_external")

ExternalProject_Add(Tetgen_external
  GIT_REPOSITORY          ${TETGEN_GIT_URL}
  GIT_TAG                 ${TETGEN_GIT_TAG}
  UPDATE_DISCONNECTED     1

  # Disconnected updates emit update and update_disconnected as siblings; under
  # -j they race on git lock files. Missed by c552399bb.
  UPDATE_COMMAND          ""

  SOURCE_DIR              ${_tetgen_src}
  BINARY_DIR              ${_tetgen_bin}

  CMAKE_GENERATOR         "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS              ${_cmake_args}
  INSTALL_COMMAND         ""     # because we set output dirs directly
)

# Obtain external project properties
ExternalProject_Get_Property(Tetgen_external SOURCE_DIR)
ExternalProject_Get_Property(Tetgen_external BINARY_DIR)
ExternalProject_Get_Property(Tetgen_external INSTALL_DIR)

# -------------------------------------------------------
# Unified exported variables (working on Windows + macOS)
# -------------------------------------------------------

set(TETGEN_INSTALL_DIR ${_tetgen_inst})
set(TETGEN_INCLUDE      ${SOURCE_DIR})
set(TETGEN_LIBRARY_DIR  ${TETGEN_INSTALL_DIR}/lib)
set(TETGEN_LIBRARY      "tet")   # matches TetGen’s CMakeLists

# Create config + use scripts for downstream projects
file(MAKE_DIRECTORY "${TETGEN_INSTALL_DIR}")

set(TETGEN_USE_FILE "${TETGEN_INSTALL_DIR}/UseTetgen.cmake")

configure_file(${SUPERBUILD_DIR}/TetgenConfig.cmake.in
               ${TETGEN_INSTALL_DIR}/TetgenConfig.cmake @ONLY)

configure_file(${SUPERBUILD_DIR}/UseTetgen.cmake
               ${TETGEN_USE_FILE} COPYONLY)

# register for find_package(Tetgen CONFIG)
set(Tetgen_DIR ${TETGEN_INSTALL_DIR} CACHE PATH "")

message(STATUS "[Tetgen_external] INSTALL_DIR=${TETGEN_INSTALL_DIR}")
message(STATUS "[Tetgen_external] USE_FILE=${TETGEN_USE_FILE}")