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


# SQLiteExternal.cmake
SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
SET(sqlite_GIT_TAG "origin/master")

ExternalProject_Add(SQLite_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/sqlite.git"
  GIT_TAG ${sqlite_GIT_TAG}
  PATCH_COMMAND ""

  # REMOVE THESE (they suppress installation)
  # INSTALL_DIR ""
  # INSTALL_COMMAND ""

  CMAKE_CACHE_ARGS
    -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DBUILD_SHARED_LIBS:BOOL=OFF
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/Externals/Install/SQLite_external

  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

# Trace the resolved install prefix (your helper will scan from here)
ExternalProject_Get_Property(SQLite_external INSTALL_DIR)
message(STATUS "[SQLite_external] INSTALL_DIR=${INSTALL_DIR}")
