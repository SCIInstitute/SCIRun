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


# QwtExternal.cmake
set_property(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# Common cache args
set(QWT_CACHE_ARGS
  "-DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5"
  "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
  "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
  "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
)

# Sanity check
if (NOT IS_DIRECTORY "${Qt_PATH}")
  message(FATAL_ERROR "Qt_PATH is invalid or not set: ${Qt_PATH}")
endif()

# Help Qwt find Qt
list(APPEND QWT_CACHE_ARGS "-DCMAKE_PREFIX_PATH:PATH=${Qt_PATH}")

# Choose Qt major and tag + give one primary hint
if (SCIRUN_QT_MAJOR STREQUAL "6")
  list(APPEND QWT_CACHE_ARGS "-DQt6_DIR:PATH=${Qt_PATH}/lib/cmake/Qt6")
  set(qwt_GIT_TAG "origin/qt6-static-6.2.0")
elseif (SCIRUN_QT_MAJOR STREQUAL "5")
  list(APPEND QWT_CACHE_ARGS "-DQt5_DIR:PATH=${Qt_PATH}/lib/cmake/Qt5")
  set(qwt_GIT_TAG "origin/qt5-static-6.1.5")
else()
  message(FATAL_ERROR "SCIRUN_QT_MAJOR must be '5' or '6'. Current value: ${SCIRUN_QT_MAJOR}")
endif()

list(APPEND QWT_CACHE_ARGS "-DSCIRUN_QT_MAJOR:STRING=${SCIRUN_QT_MAJOR}")

# Build up args in a list; only add platform/toolset when non-empty.
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

# Visual Studio generators are multi-config; don't force CMAKE_BUILD_TYPE
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

ExternalProject_Add(Qwt_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Qwt.git"
  GIT_TAG ${qwt_GIT_TAG}
  PATCH_COMMAND ""

  # Use the built-in generator handling
  CMAKE_GENERATOR "${CMAKE_GENERATOR}"
  # Only add these if they are set in the parent build
  # (CMake treats empty values as "not present")
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  # Normal CMake cache/args
  CMAKE_ARGS ${_cmake_args}

  # --- Explicit build ---
  BUILD_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --config <CONFIG>

  # --- Explicit install ---
  INSTALL_COMMAND
    ${CMAKE_COMMAND} --install <BINARY_DIR> --config <CONFIG>

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

ExternalProject_Get_Property(Qwt_external INSTALL_DIR)
message(STATUS "[Qwt_external] INSTALL_DIR=${INSTALL_DIR}")
