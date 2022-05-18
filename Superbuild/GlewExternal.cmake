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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
SET(glew_GIT_TAG "origin/master")

IF(TRAVIS_BUILD)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
ENDIF()

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Glew_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/glew.git"
  GIT_TAG ${glew_GIT_TAG}
  PATCH_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DCMAKE_CXX_FLAGS:STATIC=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STATIC=${CMAKE_C_FLAGS}
)

ExternalProject_Get_Property(Glew_external BINARY_DIR)
SET(Glew_DIR ${BINARY_DIR} CACHE PATH "")

MESSAGE(STATUS "Glew_DIR: ${Glew_DIR}")
