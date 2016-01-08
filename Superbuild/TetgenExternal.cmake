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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
ExternalProject_Add(Tetgen_external
  URL "http://www.tetgen.org/1.5/src/tetgen1.5.0.tar.gz"
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    #-DTETLIBRARY
)

ExternalProject_Get_Property(Tetgen_external SOURCE_DIR)
ExternalProject_Get_Property(Tetgen_external BINARY_DIR)
ExternalProject_Get_Property(Tetgen_external INSTALL_DIR)
SET(TETGEN_INCLUDE ${SOURCE_DIR})
SET(TETGEN_LIBRARY_DIR ${BINARY_DIR})
SET(TETGEN_USE_FILE ${INSTALL_DIR}/UseTetgen.cmake)
# see Tetgen CMakeLists.txt file
SET(TETGEN_LIBRARY "tet")
SET(Tetgen_DIR ${INSTALL_DIR} CACHE PATH "")

# Boost is special case - normally this should be handled in external library repo
CONFIGURE_FILE(${SUPERBUILD_DIR}/TetgenConfig.cmake.in ${INSTALL_DIR}/TetgenConfig.cmake @ONLY)
CONFIGURE_FILE(${SUPERBUILD_DIR}/UseTetgen.cmake ${TETGEN_USE_FILE} COPYONLY)

MESSAGE(STATUS "Tetgen_DIR: ${Tetgen_DIR}")
