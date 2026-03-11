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

# LodePngExternal.cmake (flat, no subdirs)
set_property(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

include(ExternalProject)

ExternalProject_Add(LodePng_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cibc-lodepng.git"
  GIT_TAG "origin/master"

  # Configure: generate a tiny wrapper project with the same VS generator/platform/toolset
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND}
      -D WRAPPER_SOURCE_DIR:PATH=<BINARY_DIR>/lodepng-wrapper-src
      -D WRAPPER_BUILD_DIR:PATH=<BINARY_DIR>/lodepng-wrapper-build
      -D WRAPPER_LIST_FILE:PATH=${CMAKE_CURRENT_LIST_DIR}/LodePNGWrapperProject.cmake.in
      -D LODEPNG_SRC:PATH=<SOURCE_DIR>/lodepng
      -D CMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>         # still set, but we won't use install()
      -D CMAKE_GENERATOR:STRING=${CMAKE_GENERATOR}
      -D CMAKE_GENERATOR_PLATFORM:STRING=${CMAKE_GENERATOR_PLATFORM}
      -D CMAKE_GENERATOR_TOOLSET:STRING=${CMAKE_GENERATOR_TOOLSET}
      -P ${CMAKE_CURRENT_LIST_DIR}/LodePNGWrapper_configure.cmake

  # VS is multi-config: per-config build
  BUILD_COMMAND
    ${CMAKE_COMMAND} --build "<BINARY_DIR>/lodepng-wrapper-build" --config $(Configuration)

  # ---- IMPORTANT: replace install() with an explicit copy script ----
  INSTALL_COMMAND
    ${CMAKE_COMMAND}
      -D WRAPPER_BUILD_DIR:PATH=<BINARY_DIR>/lodepng-wrapper-build
      -D WRAPPER_SOURCE_DIR:PATH=<SOURCE_DIR>            # repo root (we'll probe header in both layouts)
      -D LODEPNG_INSTALL_DIR:PATH=<INSTALL_DIR>
      -D CONFIGURATION:STRING=$(Configuration)
      -P ${CMAKE_CURRENT_LIST_DIR}/LodePNGWrapper_install.cmake

  CMAKE_CACHE_ARGS
    -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON

  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

ExternalProject_Get_Property(LodePng_external INSTALL_DIR)
message(STATUS "[LodePng_external] INSTALL_DIR=${INSTALL_DIR}")