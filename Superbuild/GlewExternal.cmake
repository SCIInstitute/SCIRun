#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

# GlewExternal.cmake (modernized + consistent + header copy into Install)
set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

set(glew_GIT_TAG "v1.0.1")

# Common CMake args
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DBUILD_SHARED_LIBS=OFF               # ensure static
  -DGLEW_USE_STATIC_LIBS=ON             # if your fork honors this

  # Redirect all outputs so install step is unnecessary for libs
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
set(_glew_src  "${CMAKE_BINARY_DIR}/Externals/Source/Glew_external")
set(_glew_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Glew_external")
set(_glew_inst "${CMAKE_BINARY_DIR}/Externals/Install/Glew_external")

ExternalProject_Add(Glew_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/glew.git"
  GIT_TAG        ${glew_GIT_TAG}
  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_glew_src}
  BINARY_DIR ${_glew_bin}

  CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS ${_cmake_args}
  INSTALL_COMMAND ""                      # we redirect outputs

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Copy headers after build (idempotent)
ExternalProject_Add_Step(Glew_external copy_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_glew_inst}/include"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${_glew_src}/include" "${_glew_inst}/include"
  DEPENDEES build
  COMMENT "[Glew_external] Copying GLEW headers to ${_glew_inst}/include"
)

# Export variables for SCIRun
set(GLEW_SOURCE_DIR  ${_glew_src})
set(GLEW_INSTALL_DIR ${_glew_inst})
set(GLEW_INCLUDE     ${GLEW_INSTALL_DIR}/include)
set(GLEW_LIBRARY_DIR ${GLEW_INSTALL_DIR}/lib)

# Pick the actual lib name produced
set(_glew_lib "")
if(EXISTS "${GLEW_LIBRARY_DIR}/glew.lib")
  set(_glew_lib "${GLEW_LIBRARY_DIR}/glew.lib")
elseif(EXISTS "${GLEW_LIBRARY_DIR}/glew32s.lib")
  set(_glew_lib "${GLEW_LIBRARY_DIR}/glew32s.lib")
elseif(EXISTS "${GLEW_LIBRARY_DIR}/glew32.lib")
  set(_glew_lib "${GLEW_LIBRARY_DIR}/glew32.lib")
endif()

# Generate a config package for GLEW so SCIRun can find GLEW::GLEW deterministically
add_custom_target(Glew_export ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GLEW_INSTALL_DIR}/lib/cmake/GLEW"
  COMMAND ${CMAKE_COMMAND}
          -D GLEW_INC:PATH="${GLEW_INCLUDE}"
          -D GLEW_LIB:PATH="${_glew_lib}"
          -P "${CMAKE_CURRENT_LIST_DIR}/GlewWriteConfig.cmake"
  DEPENDS Glew_external copy_headers
  COMMENT "[Glew_external] Writing GLEWConfig.cmake"
)

# Helper script to expand template @ONLY
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/GlewWriteConfig.cmake" "
  configure_file(\"${SUPERBUILD_DIR}/cmake/GLEWConfig.cmake.in\"
                 \"${GLEW_INSTALL_DIR}/lib/cmake/GLEW/GLEWConfig.cmake\" @ONLY)
")

message(STATUS "[Glew_external] INSTALL_DIR=${GLEW_INSTALL_DIR}")