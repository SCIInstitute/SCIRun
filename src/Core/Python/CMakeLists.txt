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
#   - Vars: Python_EXECUTABLE, Python_INCLUDE_DIRS, Python_LIBRARIES
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
# Platform-specific compile definitions (PYTHONPATH, etc.)
# ====================================================================================

# Ask Python for site-packages and use its parent (matches legacy semantics)
set(_SCIRUN_PY_QUERY
  "import sys,sysconfig; p=sysconfig.get_paths(); print((p.get('purelib') or p.get('platlib') or ''))"
)
execute_process(
  COMMAND "${Python_EXECUTABLE}" -c "${_SCIRUN_PY_QUERY}"
  OUTPUT_VARIABLE _PY_SITE
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(_PYTHON_MODULE_SEARCH_PARENT "${_PY_SITE}")
if(_PYTHON_MODULE_SEARCH_PARENT)
  get_filename_component(_PYTHON_MODULE_SEARCH_PARENT "${_PY_SITE}" DIRECTORY)
endif()

# Compute 'pythonXY' (e.g., python313) if code uses it
set(_SCIRUN_PY_NAME_QUERY "import sys; print('python{}{}'.format(sys.version_info.major, sys.version_info.minor))")
execute_process(
  COMMAND "${Python_EXECUTABLE}" -c "${_SCIRUN_PY_NAME_QUERY}"
  OUTPUT_VARIABLE _PYTHON_NAME
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Extract a representative library and its dir (used on macOS legacy code)
set(_PYTHON_LIB "")
if(Python_LIBRARIES)
  if(Python_LIBRARIES MATCHES ";")
    list(GET Python_LIBRARIES 0 _PYTHON_LIB)
  else()
    set(_PYTHON_LIB "${Python_LIBRARIES}")
  endif()
endif()
get_filename_component(_PYTHON_LIBDIR "${_PYTHON_LIB}" DIRECTORY)

# Apply defines (use add_definitions to preserve your existing behavior / L"...")
if(WIN32)
  add_definitions(
    -DPYTHONPATH=L"${_PYTHON_MODULE_SEARCH_PARENT}"
    -DPYTHONNAME=L"${_PYTHON_NAME}"
  )
elseif(APPLE)
  add_definitions(
    -DPYTHONPATH=L"${_PYTHON_MODULE_SEARCH_PARENT}"
    -DPYTHONLIBDIR=L"${_PYTHON_LIBDIR}"
    -DPYTHONLIB=L"${_PYTHON_LIB}"
  )
else()
  add_definitions(-DPYTHONPATH=L"${_PYTHON_MODULE_SEARCH_PARENT}")
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

# Option to force explicit linking on Windows (helps when auto-link is altered by other macros)
option(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK "Explicitly link Boost.Python on MSVC; otherwise rely on auto-link" OFF)

# Compute Python major.minor digits for filename patterns
# (derive from modern FindPython or legacy values)
set(_SCIRUN_PY_M "")
if(DEFINED Python_VERSION_MAJOR AND DEFINED Python_VERSION_MINOR)
  set(_SCIRUN_PY_M "${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
else()
  # Fallback: try to query from interpreter
  execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import sys; print(f'{sys.version_info.major}{sys.version_info.minor}')"
    OUTPUT_VARIABLE _SCIRUN_PY_M
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(MSVC)
  if(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK)
    # Disable autolink for Boost.Python on this target
    target_compile_definitions(Core_Python PRIVATE BOOST_PYTHON_NO_LIB)

    # Find actual lib files under Boost's lib dir (Debug has '-gd-')
    set(_bp_dbg "")
    set(_bp_rel "")

    if(DEFINED SCI_BOOST_LIBRARY_DIR)
      file(GLOB _bp_dbg
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib"
      )
      file(GLOB _bp_rel_all
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-*.lib"
      )
      foreach(_cand IN LISTS _bp_rel_all)
        if(NOT _cand MATCHES "-gd-")
          list(APPEND _bp_rel "${_cand}")
        endif()
      endforeach()
    endif()

    list(SORT _bp_dbg)  ; list(REVERSE _bp_dbg)
    list(SORT _bp_rel)  ; list(REVERSE _bp_rel)

    list(GET _bp_dbg 0 SCI_BOOST_PYTHON_LIB_DEBUG  )
    list(GET _bp_rel 0 SCI_BOOST_PYTHON_LIB_RELEASE)

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

    target_link_libraries(Core_Python PRIVATE
      $<$<CONFIG:Debug>:"${SCI_BOOST_PYTHON_LIB_DEBUG}">
      $<$<CONFIG:RelWithDebInfo>:"${SCI_BOOST_PYTHON_LIB_RELEASE}">
      $<$<CONFIG:Release>:"${SCI_BOOST_PYTHON_LIB_RELEASE}">
      $<$<CONFIG:MinSizeRel>:"${SCI_BOOST_PYTHON_LIB_RELEASE}">
    )
    message(STATUS "[Core/Python] Explicitly linking Boost.Python: ")
    message(STATUS "  Debug   : ${SCI_BOOST_PYTHON_LIB_DEBUG}")
    message(STATUS "  Release : ${SCI_BOOST_PYTHON_LIB_RELEASE}")
  else()
    # Auto-link path: do not add boost_python*.lib explicitly.
    # Ensure the Boost lib dir is on the search path (top-level does this),
    # and avoid defines like BOOST_ALL_NO_LIB / BOOST_PYTHON_NO_LIB here.
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
message(STATUS "[Core/Python] Python executable: ${Python_EXECUTABLE}")
message(STATUS "[Core/Python] Python include(s): ${Python_INCLUDE_DIRS}")
message(STATUS "[Core/Python] Python libraries : ${Python_LIBRARIES}")
message(STATUS "[Core/Python] PYTHONPATH       : ${_PYTHON_MODULE_SEARCH_PARENT}")
if(APPLE)
  message(STATUS "[Core/Python] macOS PYTHONLIBDIR: ${_PYTHON_LIBDIR}")
  message(STATUS "[Core/Python] macOS PYTHONLIB   : ${_PYTHON_LIB}")
endif()