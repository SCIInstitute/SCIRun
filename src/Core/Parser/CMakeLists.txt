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


# Sources of Core/Parser classes

set(Core_Parser_HEADERS
  ArrayMathEngine.h
  LinAlgEngine.h
  Parser.h
  ArrayMathFunctionCatalog.h
  LinAlgFunctionCatalog.h
  share.h
  ArrayMathInterpreter.h
  LinAlgInterpreter.h
)

set(Core_Parser_SRCS
  ArrayMathFunctionElement.cc
  ArrayMathFunctionTensor.cc
  ArrayMathFunctionVector.cc
  ArrayMathFunctionScalar.cc
  ArrayMathFunctionBasic.cc
  ArrayMathFunctionCatalog.cc
  ArrayMathFunctionSourceSink.cc
  ArrayMathInterpreter.cc
  ArrayMathEngine.cc
  LinAlgFunctionSourceSink.cc
  LinAlgFunctionScalar.cc
  LinAlgFunctionBasic.cc
  LinAlgFunctionCatalog.cc
  LinAlgInterpreter.cc
  LinAlgEngine.cc
  Parser.cc
)

SCIRUN_ADD_LIBRARY(Core_Parser
  ${Core_Parser_HEADERS}
  ${Core_Parser_SRCS}
)

# Export macro for this shared library (existing behavior)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(Core_Parser PRIVATE BUILD_Core_Parser)
endif()

# ==========================================================
# Teem discovery (same pattern as other fixed modules)
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
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
  )

  set(_teem_target Teem::teem)
endif()

# ------------------------------------------
# Static Teem consumption: avoid dllimport
# ------------------------------------------
# If you are using shared Teem (DLL), comment this line out.
target_compile_definitions(Core_Parser PRIVATE TEEM_STATIC)

# Existing internal deps + Teem
target_link_libraries(Core_Parser
  PRIVATE
    Core_Datatypes_Legacy_Field
    Core_Datatypes
    ${_teem_target}
)

# If Teem was built with zlib (very common for NRRD), static Teem needs zlib too.
# Prefer the config target if available, otherwise try a quiet find_package.
if(TARGET ZLIB::ZLIB)
  target_link_libraries(Core_Parser PRIVATE ZLIB::ZLIB)
else()
  find_package(ZLIB QUIET)
  if(ZLIB_FOUND)
    # Create an alias target if the package didn't provide one.
    if(NOT TARGET ZLIB::ZLIB)
      add_library(ZLIB::ZLIB INTERFACE IMPORTED)
      set_target_properties(ZLIB::ZLIB PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${ZLIB_LIBRARIES}"
      )
    endif()
    target_link_libraries(Core_Parser PRIVATE ZLIB::ZLIB)
  endif()
endif()

# Optional: Unix sometimes needs 'm' for static math, not needed on MSVC.
if(UNIX AND NOT APPLE)
  target_link_libraries(Core_Parser PRIVATE m)
endif()

SCIRUN_ADD_TEST_DIR(Tests)