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

# Cleaver2External.cmake (corrected + aligned with TetGen pattern)

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# Version tag for Cleaver2
set(cleaver2_GIT_TAG "v2.0.1")   # or your new tag like v2.0.1-scirun1

# Common CMake args
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON

  # Redirect all outputs so install step is unnecessary
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=<INSTALL_DIR>/bin

  # Multi-config (VS)
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/bin
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/bin
)

# Single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# Superbuild directories
set(_cleaver2_src  "${CMAKE_BINARY_DIR}/Externals/Source/Cleaver2_external")
set(_cleaver2_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Cleaver2_external")
set(_cleaver2_inst "${CMAKE_BINARY_DIR}/Externals/Install/Cleaver2_external")

ExternalProject_Add(Cleaver2_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Cleaver2Library.git"
  GIT_TAG        ${cleaver2_GIT_TAG}
  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_cleaver2_src}
  BINARY_DIR ${_cleaver2_bin}

  CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS ${_cmake_args}

  # Outputs already redirected -> skip install
  INSTALL_COMMAND ""
  
  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Export properties for SCIRun
ExternalProject_Get_Property(Cleaver2_external SOURCE_DIR)
set(CLEAVER2_SOURCE_DIR ${SOURCE_DIR})

set(CLEAVER2_INSTALL_DIR ${_cleaver2_inst})
set(CLEAVER2_INCLUDE     ${CLEAVER2_SOURCE_DIR}/src)   # Cleaver2 headers live in src/
set(CLEAVER2_LIBRARY_DIR ${CLEAVER2_INSTALL_DIR}/lib)
set(CLEAVER2_LIBRARY     "cleaver2")                   # matches add_library(cleaver2 ...)

message(STATUS "[Cleaver2_external] INSTALL_DIR=${CLEAVER2_INSTALL_DIR}")
