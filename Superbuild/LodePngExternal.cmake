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


# LodePngExternal.cmake
SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

ExternalProject_Add(LodePng_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cibc-lodepng.git"
  GIT_TAG "origin/master"
  PATCH_COMMAND ""

  # LodePNG has no configure/build step.
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""

  # Install manually: headers + source (library is header-only/inline or single .cpp)
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E make_directory
      "${CMAKE_BINARY_DIR}/Externals/Install/LodePng_external/include/lodepng"
    &&
    ${CMAKE_COMMAND} -E copy_directory
      "<SOURCE_DIR>/lodepng"
      "${CMAKE_BINARY_DIR}/Externals/Install/LodePng_external/include/lodepng"

  CMAKE_CACHE_ARGS
    -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON

  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

# Debug output for install prefix
ExternalProject_Get_Property(LodePng_external INSTALL_DIR)
message(STATUS "[LodePng_external] INSTALL_DIR=${INSTALL_DIR}")
