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


# FreetypeExternal.cmake (modernized + consistent)

# FreetypeExternal.cmake — use installed zlib via CMAKE_PREFIX_PATH

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

set(freetype_GIT_TAG "v2.14.1")

# Common CMake args for FreeType
# NOTE:
#  - We no longer need to redirect all output dirs when using find_package().
#  - We pass CMAKE_PREFIX_PATH to point at the installed zlib prefix.
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

  # Make FreeType use the zlib we installed in its own external
  -DCMAKE_PREFIX_PATH=${ZLIB_INSTALL_DIR}
  -DFT_REQUIRE_ZLIB=ON

  # Start minimal to avoid surprise dependencies during bring-up
  -DFT_DISABLE_BZIP2=ON
  -DFT_DISABLE_PNG=ON
  -DFT_DISABLE_BROTLI=ON
  -DFT_DISABLE_HARFBUZZ=ON
)

# Single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# Superbuild directories
set(_freetype_src  "${CMAKE_BINARY_DIR}/Externals/Source/Freetype_external")
set(_freetype_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Freetype_external")
set(_freetype_inst "${CMAKE_BINARY_DIR}/Externals/Install/Freetype_external")

ExternalProject_Add(Freetype_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/freetype.git"
  GIT_TAG        ${freetype_GIT_TAG}
  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_freetype_src}
  BINARY_DIR ${_freetype_bin}

  CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS ${_cmake_args}

  # Ensure zlib is fully built & installed before configuring FreeType
  DEPENDS Zlib_external

  # Keep FreeType in "no-install" mode for now (you can enable later if desired)
  INSTALL_COMMAND ""

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Export variables for SCIRun (consumer side)
set(FREETYPE_SOURCE_DIR  ${_freetype_src})
set(FREETYPE_INSTALL_DIR ${_freetype_inst})

# Public headers for consumers are under source/include
set(FREETYPE_INCLUDE     ${FREETYPE_SOURCE_DIR}/include)

# Libraries land here if you later enable FreeType's install;
# with INSTALL_COMMAND "" it remains empty, but we keep the var for consistency.
set(FREETYPE_LIBRARY_DIR ${FREETYPE_INSTALL_DIR}/lib)

# Basename (actual file becomes freetype.lib / libfreetype.a / libfreetype.{so,dylib})
set(FREETYPE_LIBRARY     "freetype")

message(STATUS "[Freetype_external] INSTALL_DIR=${FREETYPE_INSTALL_DIR}")