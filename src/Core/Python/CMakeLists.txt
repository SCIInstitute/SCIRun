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

# ---- Core/Python CMake ----

# --- Sources ---
set(Core_Python_SRCS
  PythonInterpreter.cc
  PythonDatatypeConverter.cc
)

set(Core_Python_HEADERS
  PythonInterpreter.h
  PythonDatatypeConverter.h
  share.h
)

# Library target (uses project macro from top-level)
SCIRUN_ADD_LIBRARY(Core_Python
  ${Core_Python_HEADERS}
  ${Core_Python_SRCS}
)

# Preserve original shared define behavior
if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Core_Python)
endif()

# If Python enabled, expose BUILD_WITH_PYTHON so dependents (and sources) can #ifdef code
if(BUILD_WITH_PYTHON)
  target_compile_definitions(Core_Python PUBLIC BUILD_WITH_PYTHON)
else()
  message(STATUS "BUILD_WITH_PYTHON=OFF, Core_Python will be built in no-Python mode (code must #ifdef accordingly).")
endif()

# --- Unit tests ---
SCIRUN_ADD_TEST_DIR(Tests)

# ====================================================================================
# Python detection (modern-first, robust fallbacks)
# ====================================================================================
# Result:
#   - Imported target: Python::Python
#   - Vars: Python_EXECUTABLE, Python_INCLUDE_DIRS, Python_LIBRARIES, Python_VERSION_*
set(_SCIRUN_PY_OK FALSE)

# Prefer FindPython (CMake ≥ 3.15)
find_package(Python QUIET COMPONENTS Interpreter Development)
if(Python_Interpreter_FOUND AND Python_Development_FOUND)
  set(_SCIRUN_PY_OK TRUE)
endif()

# Fallback: FindPython3 (CMake ≥ 3.12)
if(NOT _SCIRUN_PY_OK)
  find_package(Python3 QUIET COMPONENTS Interpreter Development)
  if(Python3_Interpreter_FOUND AND Python3_Development_FOUND)
    set(Python_EXECUTABLE   "${Python3_EXECUTABLE}")
    set(Python_INCLUDE_DIRS "${Python3_INCLUDE_DIRS}")
    set(Python_LIBRARIES    "${Python3_LIBRARIES}")
    if(TARGET Python3::Python AND NOT TARGET Python::Python)
      add_library(Python::Python ALIAS Python3::Python)
    endif()
    set(_SCIRUN_PY_OK TRUE)
  endif()
endif()

# Legacy fallback (older CMake)
if(NOT _SCIRUN_PY_OK)
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)
  set(Python_EXECUTABLE   "${PYTHON_EXECUTABLE}")
  set(Python_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
  set(Python_LIBRARIES    "${PYTHON_LIBRARIES}")
  if(NOT TARGET Python::Python)
    add_library(Python::Python UNKNOWN IMPORTED)
    set_target_properties(Python::Python PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Python_INCLUDE_DIRS}"
    )
    if(WIN32)
      set_target_properties(Python::Python PROPERTIES IMPORTED_IMPLIB "${Python_LIBRARIES}")
    else()
      set_target_properties(Python::Python PROPERTIES IMPORTED_LOCATION "${Python_LIBRARIES}")
    endif()
  endif()
  set(_SCIRUN_PY_OK TRUE)
endif()

if(NOT _SCIRUN_PY_OK)
  message(FATAL_ERROR "Python (Interpreter + Development) not found; cannot build Core_Python.")
endif()

# ====================================================================================
# Platform-specific compile definitions (PYTHONPATH, etc.) -- robust version
# ====================================================================================

# Compute "pythonXY" (e.g., python313) from detected version variables
set(_PYTHON_NAME "")
if(DEFINED Python_VERSION_MAJOR AND DEFINED Python_VERSION_MINOR)
  set(_PYTHON_NAME "python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
endif()

# Compute parent of site-packages (purelib/platlib) using Python (safe: fully quoted string)
# Note: semicolons inside the quoted string are part of Python code and NOT parsed by CMake.
set(_SCIRUN_PY_QUERY
  "import sysconfig; p=sysconfig.get_paths(); print((p.get('purelib') or p.get('platlib') or ''))"
)
set(_PY_SITE "")
execute_process(
  COMMAND "${Python_EXECUTABLE}" -c "${_SCIRUN_PY_QUERY}"
  OUTPUT_VARIABLE _PY_SITE
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Parent folder of site-packages
set(_PYTHON_MODULE_SEARCH_PARENT "")
if(_PY_SITE)
  get_filename_component(_PYTHON_MODULE_SEARCH_PARENT "${_PY_SITE}" DIRECTORY)
  # Normalize slashes so the value is safe in a C string literal L"..."
  file(TO_CMAKE_PATH "${_PYTHON_MODULE_SEARCH_PARENT}" _PYTHON_MODULE_SEARCH_PARENT)
endif()

# Extract a representative Python lib path and its dir (used on macOS legacy code)
set(_PYTHON_LIB "")
if(Python_LIBRARIES)
  if(Python_LIBRARIES MATCHES ";")
    list(GET Python_LIBRARIES 0 _PYTHON_LIB)
  else()
    set(_PYTHON_LIB "${Python_LIBRARIES}")
  endif()
endif()
get_filename_component(_PYTHON_LIBDIR "${_PYTHON_LIB}" DIRECTORY)

# ALWAYS define these macros so sources never see an undefined identifier.
# We use target_compile_definitions and inject *wide* string literals L"...".
if(WIN32)
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
    "PYTHONNAME=L\"${_PYTHON_NAME}\""
  )
elseif(APPLE)
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
    "PYTHONLIBDIR=L\"${_PYTHON_LIBDIR}\""
    "PYTHONLIB=L\"${_PYTHON_LIB}\""
  )
else()
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
  )
endif()

# ====================================================================================
# Link dependencies
# ====================================================================================

# Always link the Python imported target
target_link_libraries(Core_Python
  PRIVATE
    Python::Python
    ${SCI_BOOST_LIBRARY}
    Core_Matlab
    Core_Datatypes
    Core_Datatypes_Legacy_Field
)

# ------------------------------------------------------------------------------------
# Boost.Python handling
# ------------------------------------------------------------------------------------
# Strategy:
#   - On MSVC (Windows): Prefer Boost's MSVC auto-link. DO NOT name 'boost_python...'
#     explicitly; the headers inject the correct /DEFAULTLIB:libboost_python....
#   - Provide an opt-in to link explicitly by detecting the actual .lib names built by b2.
#   - On non-Windows, explicitly link Boost.Python (no auto-link there).

option(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK "Explicitly link Boost.Python on MSVC; otherwise rely on auto-link" OFF)

# Compute Python major.minor digits for filename patterns
set(_SCIRUN_PY_M "")
if(DEFINED Python_VERSION_MAJOR AND DEFINED Python_VERSION_MINOR)
  set(_SCIRUN_PY_M "${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
endif()

if(MSVC)
  if(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK)
    # Disable autolink for Boost.Python on this target
    target_compile_definitions(Core_Python PRIVATE BOOST_PYTHON_NO_LIB)

    # Find actual lib files under Boost's lib dir (Debug has '-gd-')
    set(SCI_BOOST_PYTHON_LIB_DEBUG "")
    set(SCI_BOOST_PYTHON_LIB_RELEASE "")

    if(DEFINED SCI_BOOST_LIBRARY_DIR AND EXISTS "${SCI_BOOST_LIBRARY_DIR}")
      file(GLOB _bp_dbg
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib"
      )
      file(GLOB _bp_rel_all
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-*.lib"
      )
      set(_bp_rel "")
      foreach(_cand IN LISTS _bp_rel_all)
        if(NOT _cand MATCHES "-gd-")
          list(APPEND _bp_rel "${_cand}")
        endif()
      endforeach()

      if(_bp_dbg)
        list(SORT _bp_dbg)
        list(REVERSE _bp_dbg)
        list(GET _bp_dbg 0 SCI_BOOST_PYTHON_LIB_DEBUG)
      endif()
      if(_bp_rel)
        list(SORT _bp_rel)
        list(REVERSE _bp_rel)
        list(GET _bp_rel 0 SCI_BOOST_PYTHON_LIB_RELEASE)
      endif()
    endif()

    if(NOT SCI_BOOST_PYTHON_LIB_DEBUG OR NOT EXISTS "${SCI_BOOST_PYTHON_LIB_DEBUG}")
      message(FATAL_ERROR
        "Boost.Python (Debug) not found under '${SCI_BOOST_LIBRARY_DIR}'. "
        "Expected 'libboost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib'")
    endif()
    if(NOT SCI_BOOST_PYTHON_LIB_RELEASE OR NOT EXISTS "${SCI_BOOST_PYTHON_LIB_RELEASE}")
      message(FATAL_ERROR
        "Boost.Python (Release) not found under '${SCI_BOOST_LIBRARY_DIR}'. "
        "Expected 'libboost_python${_SCIRUN_PY_M}-vc*-mt-*.lib' (no -gd-)")
    endif()

    # Per-config link of the actual .lib files
    target_link_libraries(Core_Python PRIVATE
      debug     "${SCI_BOOST_PYTHON_LIB_DEBUG}"
      optimized "${SCI_BOOST_PYTHON_LIB_RELEASE}"
    )
    message(STATUS "[Core/Python] Explicitly linking Boost.Python:")
    message(STATUS "  Debug   : ${SCI_BOOST_PYTHON_LIB_DEBUG}")
    message(STATUS "  Release : ${SCI_BOOST_PYTHON_LIB_RELEASE}")
  else()
    # Auto-link path: do not add boost_python*.lib explicitly.
    # Ensure the Boost lib dir is in the search path (top-level handles it).
    # Avoid defines like BOOST_ALL_NO_LIB / BOOST_PYTHON_NO_LIB here.
    message(STATUS "[Core/Python] Using MSVC autolink for Boost.Python (recommended).")
  endif()
else()
  # Non-Windows: explicitly link Boost.Python
  set(_bp_names
    "boost_python${_SCIRUN_PY_M}"
    "boost_python3"
    "boost_python"
  )
  set(_bp_lib "")

  # Prefer the superbuild lib dir if present
  if(DEFINED SCI_BOOST_LIBRARY_DIR AND EXISTS "${SCI_BOOST_LIBRARY_DIR}")
    find_library(BOOST_PYTHON_LIB
      NAMES ${_bp_names}
      PATHS "${SCI_BOOST_LIBRARY_DIR}" "${SCI_BOOST_PREFIX}/lib" "${SCI_BOOST_PREFIX}/lib64"
      NO_DEFAULT_PATH
    )
    set(_bp_lib "${BOOST_PYTHON_LIB}")
  endif()

  # Fallback to default search paths if still not found
  if(NOT _bp_lib)
    find_library(BOOST_PYTHON_LIB NAMES ${_bp_names})
    set(_bp_lib "${BOOST_PYTHON_LIB}")
  endif()

  if(_bp_lib)
    target_link_libraries(Core_Python PRIVATE "${_bp_lib}")
    message(STATUS "[Core/Python] Linking Boost.Python: ${_bp_lib}")
  else()
    message(WARNING
      "[Core/Python] Boost.Python library not found on this platform. "
      "Link may fail if Boost.Python symbols are used.")
  endif()
endif()

# Debug-only macro (preserve your behavior)
set_target_properties(Core_Python
  PROPERTIES
    COMPILE_DEFINITIONS_DEBUG "BOOST_DEBUG_PYTHON"
)

# Helpful diagnostics
message(STATUS "[Core/Python] Python executable : ${Python_EXECUTABLE}")
message(STATUS "[Core/Python] Python include(s) : ${Python_INCLUDE_DIRS}")
message(STATUS "[Core/Python] Python libraries  : ${Python_LIBRARIES}")
message(STATUS "[Core/Python] PYTHONNAME       : ${_PYTHON_NAME}")
message(STATUS "[Core/Python] PYTHONPATH       : ${_PYTHON_MODULE_SEARCH_PARENT}")