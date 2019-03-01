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
  SET(TBB_URL "https://github.com/01org/tbb/releases/download/2019_U3/tbb2019_20181203oss_mac.tgz")
ELSE()
  IF(WIN32)
    SET(TBB_URL "https://github.com/01org/tbb/releases/download/2019_U3/tbb2019_20181203oss_win.zip")
  ELSE() # Linux
    SET(TBB_URL "https://github.com/01org/tbb/releases/download/2019_U3/tbb2019_20181203oss_lin.tgz")
  ENDIF()
ENDIF()

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
ExternalProject_Add(Tbb_external
  URL ${TBB_URL}
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
)

ExternalProject_Get_Property(Tbb_external SOURCE_DIR)
SET(Tbb_extra_dir_name "tbb2019_20181203oss")
SET(Tbb_DIR "${SOURCE_DIR}/${Tbb_extra_dir_name}" CACHE PATH "")

MESSAGE(STATUS "Tbb_DIR: ${Tbb_DIR}")
