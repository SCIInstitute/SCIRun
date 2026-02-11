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
# Core/GeometryPrimitives CMake
# ================================

# --- Sources & headers ---
set(Core_GeometryPrimitives_SRCS
  BBoxBase.cc
  BBox.cc
  OrientedBBox.cc
  CompGeom.cc
  Plane.cc
  Point.cc
  SearchGridT.cc
  Tensor.cc
  Transform.cc
  Vector.cc
)

set(Core_GeometryPrimitives_HEADERS
  BBoxBase.h
  BBox.h
  OrientedBBox.h
  CompGeom.h
  GeomFwd.h
  Plane.h
  Point.h
  PointVectorOperators.h
  SearchGridT.h
  Tensor.h
  Transform.h
  Vector.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_Geometry_Primitives
  ${Core_GeometryPrimitives_HEADERS}
  ${Core_GeometryPrimitives_SRCS}
)

if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Core_Geometry_Primitives)
endif()

# ==========================================================
# Teem discovery (provides tenEigensolve_f used by Tensor.cc)
# ==========================================================

# Preferred: Teem config package if available
# (You can create/export one from the superbuild later; this handles that case automatically.)
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

  # Fallback: search by name with hints if not found yet
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
  )

  set(_teem_target Teem::teem)
endif()

# ============================================
# ZLIB (needed by Teem; useful for static Teem)
# ============================================

# Try to use a config/module package first.
# Your superbuild already exports ZLIB_ROOT & ZLIB_DIR and sets CMAKE_PREFIX_PATH, so this should work.
find_package(ZLIB QUIET)

# If ZLIB::ZLIB target isn't available but include/lib hints exist, create one.
if(NOT TARGET ZLIB::ZLIB)
  # Common superbuild exports include ZLIB_INCLUDE_DIR and a library in <prefix>/lib[64].
  # Try to deduce them if not set by find_package.
  if(NOT ZLIB_INCLUDE_DIR AND DEFINED ZLIB_ROOT AND EXISTS "${ZLIB_ROOT}/include")
    set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT}/include")
  endif()

  # Try typical library file names if not found by package
  set(_zlib_lib "")
  # Prefer lib64 when present
  if(DEFINED ZLIB_ROOT AND EXISTS "${ZLIB_ROOT}/lib64")
    set(_zlib_libdir "${ZLIB_ROOT}/lib64")
  elseif(DEFINED ZLIB_ROOT AND EXISTS "${ZLIB_ROOT}/lib")
    set(_zlib_libdir "${ZLIB_ROOT}/lib")
  endif()

  if(_zlib_libdir)
    if(WIN32)
      if(EXISTS "${_zlib_libdir}/zlib.lib")
        set(_zlib_lib "${_zlib_libdir}/zlib.lib")
      elseif(EXISTS "${_zlib_libdir}/zlibstatic.lib")
        set(_zlib_lib "${_zlib_libdir}/zlibstatic.lib")
      endif()
    elseif(APPLE)
      if(EXISTS "${_zlib_libdir}/libz.dylib")
        set(_zlib_lib "${_zlib_libdir}/libz.dylib")
      elseif(EXISTS "${_zlib_libdir}/libz.a")
        set(_zlib_lib "${_zlib_libdir}/libz.a")
      endif()
    else()
      if(EXISTS "${_zlib_libdir}/libz.so")
        set(_zlib_lib "${_zlib_libdir}/libz.so")
      elseif(EXISTS "${_zlib_libdir}/libz.a")
        set(_zlib_lib "${_zlib_libdir}/libz.a")
      endif()
    endif()
  endif()

  if(NOT _zlib_lib)
    # Last resort: search by name with hints
    find_library(ZLIB_LIBRARY
      NAMES z zlib zlibstatic
      HINTS "${_zlib_libdir}" "${CMAKE_PREFIX_PATH}" "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
    )
    if(ZLIB_LIBRARY)
      set(_zlib_lib "${ZLIB_LIBRARY}")
    endif()
  endif()

  if(_zlib_lib AND ZLIB_INCLUDE_DIR)
    add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
    set_target_properties(ZLIB::ZLIB PROPERTIES
      IMPORTED_LOCATION "${_zlib_lib}"
      INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}"
    )
  endif()
endif()

# ===========================
# Link libraries for this lib
# ===========================
target_link_libraries(Core_Geometry_Primitives
  PRIVATE
    Core_Math
    Core_Util_Legacy
    Core_Persistent
    ${_teem_target}
)

# If ZLIB::ZLIB exists, add it (especially useful when Teem is static and needs zlib symbols)
if(TARGET ZLIB::ZLIB)
  target_link_libraries(Core_Geometry_Primitives PRIVATE ZLIB::ZLIB)
endif()

# ===========================
# Tests
# ===========================
SCIRUN_ADD_TEST_DIR(Tests)