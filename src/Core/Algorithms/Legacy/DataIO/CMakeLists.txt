#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
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
#

# Sources of Core/Algorithms/Legacy/DataIO classes

set(Core_Algorithms_Legacy_DataIO_SRCS
  DataIOAlgo.cc
  #G3DToFieldReader.cc
  ObjToFieldReader.cc
  VTKToTriSurfReader.cc
  TriSurfSTLBinaryConverter.cc
  TriSurfSTLASCIIConverter.cc
  STLUtils.cc
)

set(Core_Algorithms_Legacy_DataIO_HEADERS
  #G3DToFieldReader.h
  ObjToFieldReader.h
  VTKToTriSurfReader.h
  DataIOAlgo.h
  TriSurfSTLBinaryConverter.h
  TriSurfSTLASCIIConverter.h
  STLUtils.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_Algorithms_Legacy_DataIO
  ${Core_Algorithms_Legacy_DataIO_SRCS}
  ${Core_Algorithms_Legacy_DataIO_HEADERS}
)

# Export macro for this shared library (existing behavior)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(Core_Algorithms_Legacy_DataIO PRIVATE BUILD_Core_Algorithms_Legacy_DataIO)
endif()

# ==========================================================
# Teem discovery (same pattern as Core_Algorithms_Legacy_Converter)
# ==========================================================
# Prefer a config package if available
find_package(Teem CONFIG QUIET)

if(TARGET Teem::teem)
  set(_teem_target Teem::teem)
else()
  # Fallback to superbuild exports
  # Expect these to be set by the superbuild toolchain or cache
  if(NOT Teem_INCLUDE_DIR OR NOT Teem_LIB_DIR)
    message(FATAL_ERROR
      "Teem not configured: need Teem_INCLUDE_DIR and Teem_LIB_DIR from the superbuild.\n"
      "Teem_INCLUDE_DIR='${Teem_INCLUDE_DIR}'  Teem_LIB_DIR='${Teem_LIB_DIR}'")
  endif()

  # Try to resolve the actual library file
  set(_teem_lib "")
  if(WIN32)
    if(EXISTS "${Teem_LIB_DIR}/teem.lib")
      set(_teem_lib "${Teem_LIB_DIR}/teem.lib")
    elseif(EXISTS "${Teem_LIB_DIR}/libteem.lib")
      set(_teem_lib "${Teem_LIB_DIR}/libteem.lib")
    endif()
  elseif(APPLE)
    if(EXISTS "${Teem_LIB_DIR}/libteem.dylib")
      set(_teem_lib "${Teem_LIB_DIR}/libteem.dylib")
