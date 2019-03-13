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

IF(APPLE)
  SET(EMBREE_URL "https://github.com/embree/embree/releases/download/v3.4.0/embree-3.4.0.x86_64.macosx.tar.gz")
ELSE()
  IF(WIN32)
    SET(EMBREE_URL "https://github.com/embree/embree/releases/download/v3.4.0/embree-3.4.0.x64.vc14.windows.zip")
  ELSE() # Linux
    SET(EMBREE_URL "https://github.com/embree/embree/releases/download/v3.4.0/embree-3.4.0.x86_64.linux.tar.gz")
  ENDIF()
ENDIF()

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
ExternalProject_Add(Embree_external
  URL ${EMBREE_URL}
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
)

ExternalProject_Get_Property(Embree_external SOURCE_DIR)
SET(Embree_DIR ${SOURCE_DIR} CACHE PATH "")

MESSAGE(STATUS "Embree_DIR: ${Embree_DIR}")
