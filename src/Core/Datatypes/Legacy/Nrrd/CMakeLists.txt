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

# ================================
# Core/Datatypes/Legacy/Nrrd
# ================================

set(Core_Datatypes_Legacy_Nrrd_HEADERS
  NrrdData.h
  share.h
)

set(Core_Datatypes_Legacy_Nrrd_SRCS
  NrrdData.cc
)

SCIRUN_ADD_LIBRARY(Core_Datatypes_Legacy_Nrrd
  ${Core_Datatypes_Legacy_Nrrd_SRCS}
  ${Core_Datatypes_Legacy_Nrrd_HEADERS}
)

# Export macro for this shared lib (kept behavior)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(Core_Datatypes_Legacy_Nrrd PRIVATE BUILD_Core_Datatypes_Legacy_Nrrd)
endif()

# ==========================================
# Teem discovery (same pattern as Geometry)
# ==========================================
# Preferred: Teem config package if available
find_package(Teem CONFIG QUIET)

if(TARGET Teem::teem)
  set(_teem_target Teem::teem)
else()
  # Fallback: Use superbuild hints Teem_INCLUDE_DIR / Teem_LIB_DIR
  if(NOT Teem_INCLUDE_DIR OR NOT Teem_LIB_DIR)
    message(FATAL_ERROR
      "Teem not configured: need Teem_INCLUDE_DIR and Teem_LIB_DIR from the superbuild.\n"
      "Teem_INCLUDE_DIR='${Teem_INCLUDE_DIR}'  Teem_LIB_DIR='${Teem_LIB_DIR}'")
  endif()

  # Try canonical filenames first by platform
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
    elseif(EXISTS "${Teem_LIB_DIR}/libteem.a")
      set(_teem_lib "${Teem_LIB_DIR}/libteem.a")
    endif()
  else()
    if(EXISTS "${Teem_LIB_DIR}/libteem.so")
      set(_teem_lib "${Teem_LIB_DIR}/libteem.so")
    elseif(EXISTS "${Teem_LIB_DIR}/libteem.a")
      set(_teem_lib "${Teem_LIB_DIR}/libteem.a")
    endif()
  endif()

  # Fallback: search by name with hints
  if(NOT _teem_lib)
    find_library(TEEM_LIBRARY
      NAMES teem libteem
      HINTS "${Teem_LIB_DIR}" "${CMAKE_PREFIX_PATH}" "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
    )
    if(TEEM_LIBRARY)
      set(_teem_lib "${TEEM_LIBRARY}")
    endif()
  endif()

  if(NOT _teem_lib OR NOT EXISTS "${_teem_lib}")
    message(FATAL_ERROR
      "Teem library not found. Looked under '${Teem_LIB_DIR}'.\n"
      "Teem_INCLUDE_DIR='${Teem_INCLUDE_DIR}'  Teem_LIB_DIR='${Teem_LIB_DIR}'\n"
      "Ensure the Teem external installs libteem/teem.")
  endif()

  add_library(Teem::teem UNKNOWN IMPORTED)
  set_target_properties(Teem::teem PROPERTIES
    IMPORTED_LOCATION "${_teem_lib}"
    INTERFACE_INCLUDE_DIRECTORIES "${Teem_INCLUDE_DIR}"
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"  # static C lib; helps some generators
  )

  set(_teem_target Teem::teem)
endif()

# ------------------------------------------
# Static Teem consumption: avoid dllimport
# ------------------------------------------
target_compile_definitions(Core_Datatypes_Legacy_Nrrd PRIVATE TEEM_STATIC)

# Existing internal deps
target_link_libraries(Core_Datatypes_Legacy_Nrrd
  PRIVATE
    Core_Exceptions_Legacy
    Core_Geometry_Primitives
    Core_Math
    Core_Thread
    Core_Utils
    Core_Util_Legacy
    Core_Datatypes
    Core_Datatypes_Legacy_Base
    ${_teem_target}
)

# If zlib was enabled in Teem (common for NRRD), static Teem needs zlib symbols
if(TARGET ZLIB::ZLIB)
  target_link_libraries(Core_Datatypes_Legacy_Nrrd PRIVATE ZLIB::ZLIB)
endif()

#SCIRUN_ADD_TEST_DIR(Tests)