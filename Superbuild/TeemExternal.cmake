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


# TeemExternal.cmake — wire Teem to the installed zlib

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

set(teem_GIT_TAG "v1.11.1")
set(teem_DEPENDENCIES Zlib_external)

# Superbuild directories
set(_teem_src  "${CMAKE_BINARY_DIR}/Externals/Source/Teem_external")
set(_teem_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Teem_external")
set(_teem_inst "${CMAKE_BINARY_DIR}/Externals/Install/Teem_external")

# Teem CMake args (no output redirection needed)
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

  # Make Teem find our installed zlib
  -DCMAKE_PREFIX_PATH=${ZLIB_INSTALL_DIR}
  -DZLIB_ROOT=${ZLIB_INSTALL_DIR}
  -DZLIB_INCLUDE_DIR=${ZLIB_INSTALL_DIR}/include

  # Start minimal: disable optional deps until zlib link works
  # (Uncomment if these options exist in your Teem tree)
   -DTEEM_ZLIB=ON
   -DTEEM_PNG=OFF
   -DTEEM_BZIP2=OFF
   -DTEEM_FFTW=OFF
)

if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

ExternalProject_Add(Teem_external
  DEPENDS ${teem_DEPENDENCIES}

  GIT_REPOSITORY "https://github.com/CIBC-Internal/teem.git"
  GIT_TAG        ${teem_GIT_TAG}
  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_teem_src}
  BINARY_DIR ${_teem_bin}

  CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  # Use CMAKE_CACHE_ARGS so Teem's *own* cache sees the hints
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

    -DZlib_DIR:PATH=${Zlib_DIR}

    -DCMAKE_PREFIX_PATH:PATH=${ZLIB_INSTALL_DIR}
    -DZLIB_ROOT:PATH=${ZLIB_INSTALL_DIR}
    -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INSTALL_DIR}/include
    -DZLIB_USE_STATIC_LIBS:BOOL=ON   # optional

    # If Teem exposes switches, keep them minimal while bringing up:
    # -DTEEM_ZLIB:BOOL=ON
    # -DTEEM_PNG:BOOL=OFF
    # -DTEEM_BZIP2:BOOL=OFF
    # -DTEEM_FFTW:BOOL=OFF

  DEPENDS Zlib_external
  INSTALL_COMMAND ""

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Make Teem's configure wait for zlib artifacts to exist
ExternalProject_Add_Step(Teem_external wait_for_zlib
  COMMAND ${CMAKE_COMMAND} -E echo "Waiting for zlib artifacts..."
  DEPENDEES download
  DEPENDERS configure
  DEPENDS
    "${ZLIB_INSTALL_DIR}/include/zlib.h"
    "${ZLIB_INSTALL_DIR}/lib"
)

# Export variables for SCIRun (consumer side)
set(TEEM_SOURCE_DIR  ${_teem_src})
set(TEEM_INSTALL_DIR ${_teem_inst})

# If you keep Teem in no-install mode for now, public headers remain in source:
set(TEEM_INCLUDE     ${TEEM_SOURCE_DIR}/src)
set(TEEM_LIBRARY_DIR ${TEEM_INSTALL_DIR}/lib)   # will populate when/if you enable install
set(TEEM_LIBRARY     "teem")

message(STATUS "[Teem_external] INSTALL_DIR=${TEEM_INSTALL_DIR}")