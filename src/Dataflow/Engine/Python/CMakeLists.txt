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


# ==========================================
# Dataflow/Engine/Python CMakeLists.txt
# ==========================================

# Optional: if you want to skip this target when Python support is OFF.
# Comment this block out if you prefer to always configure it.
if(NOT BUILD_WITH_PYTHON)
  message(STATUS "[Dataflow/Engine/Python] Skipping SCIRunPythonAPI (BUILD_WITH_PYTHON=OFF)")
  return()
endif()

# --- Sources & headers ---
set(SCIRunPythonAPI_SRCS
  NetworkEditorPythonAPI.cc
)

set(SCIRunPythonAPI_HEADERS
  NetworkEditorPythonAPI.h
  NetworkEditorPythonInterface.h
  SCIRunPythonModule.h
  share.h
)

# --- Library target ---
SCIRUN_ADD_LIBRARY(SCIRunPythonAPI
  ${SCIRunPythonAPI_HEADERS}
  ${SCIRunPythonAPI_SRCS}
)

# ====================================================================================
# Cross-platform Python detection with superbuild override (Windows multi-config)
# ====================================================================================

# Expect these (or similar) to be passed from the superbuild. Adjust names if yours differ:
#   PY_EXT_LIB_DIR:  e.g., E:/PROJECTS/SCIRun/build/Externals/Source/Python_external/PCbuild/amd64
#   PY_INCLUDE_DIR:  e.g., E:/PROJECTS/SCIRun/build/Externals/Source/Python_external/Include
#   PY_EXECUTABLE:   e.g., E:/PROJECTS/SCIRun/build/Externals/Source/Python_external/python.exe (optional)

# Try modern find first (harmless even if we will override later)
set(_SCIRUN_API_PY_OK FALSE)
find_package(Python QUIET COMPONENTS Interpreter Development)
if(Python_Interpreter_FOUND AND Python_Development_FOUND)
  set(_SCIRUN_API_PY_OK TRUE)
else()
  find_package(Python3 QUIET COMPONENTS Interpreter Development)
  if(Python3_Interpreter_FOUND AND Python3_Development_FOUND)
    # normalize to Python::Python
    if(TARGET Python3::Python AND NOT TARGET Python::Python)
      add_library(Python::Python ALIAS Python3::Python)
    endif()
    set(_SCIRUN_API_PY_OK TRUE)
  endif()
endif()

# --- Superbuild override on Windows: construct the imported target from explicit paths
# Accept either the new variables (PY_EXT_LIB_DIR/PY_INCLUDE_DIR) or the legacy ones
# (PYTHON_LIBRARY_DEBUG/RELEASE, PYTHON_INCLUDE_DIR). Prefer the new ones when present.

if(MSVC)
  # Derived names for Python 3.13 (adjust as needed)
  set(_PY_DEBUG_LIB_NAME   "python313_d.lib")
  set(_PY_RELEASE_LIB_NAME "python313.lib")

  # Prefer new-style
  set(_SB_LIB_DIR   "${PY_EXT_LIB_DIR}")
  set(_SB_INC_DIR   "${PY_INCLUDE_DIR}")

  # Fallback to legacy variables if new-style not provided
  if(NOT _SB_LIB_DIR)
    # Try to infer from the legacy file paths
    if(PYTHON_LIBRARY_DEBUG)
      get_filename_component(_SB_LIB_DIR "${PYTHON_LIBRARY_DEBUG}" DIRECTORY)
    elseif(PYTHON_LIBRARY_RELEASE)
      get_filename_component(_SB_LIB_DIR "${PYTHON_LIBRARY_RELEASE}" DIRECTORY)
    endif()
  endif()

  if(NOT _SB_INC_DIR)
    if(PYTHON_INCLUDE_DIR)
      set(_SB_INC_DIR "${PYTHON_INCLUDE_DIR}")
    elseif(Python_INCLUDE_DIRS)
      # FindPython may pass a list; use the first element as base include
      list(GET Python_INCLUDE_DIRS 0 _SB_INC_DIR)
    endif()
  endif()

  # As last resort, try known layout relative to the inner build
  if(NOT _SB_LIB_DIR)
    set(_SB_LIB_DIR "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external/PCbuild/amd64")
  endif()
  if(NOT _SB_INC_DIR)
    set(_SB_INC_DIR "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external/Include")
  endif()

  # Resolve per-config import libraries
  set(_py_dbg_lib "")
  set(_py_rel_lib "")

  # If explicit files were provided by superbuild, use them
  if(PYTHON_LIBRARY_DEBUG AND EXISTS "${PYTHON_LIBRARY_DEBUG}")
    set(_py_dbg_lib "${PYTHON_LIBRARY_DEBUG}")
  endif()
  if(PYTHON_LIBRARY_RELEASE AND EXISTS "${PYTHON_LIBRARY_RELEASE}")
    set(_py_rel_lib "${PYTHON_LIBRARY_RELEASE}")
  endif()

  # Otherwise, construct by name under the lib dir
  if(NOT _py_dbg_lib)
    set(_cand "${_SB_LIB_DIR}/${_PY_DEBUG_LIB_NAME}")
    if(EXISTS "${_cand}")
      set(_py_dbg_lib "${_cand}")
    endif()
  endif()
  if(NOT _py_rel_lib)
    set(_cand "${_SB_LIB_DIR}/${_PY_RELEASE_LIB_NAME}")
    if(EXISTS "${_cand}")
      set(_py_rel_lib "${_cand}")
    endif()
  endif()

  # Build/update Python::Python imported target pointing at superbuild libs
  if(NOT TARGET Python::Python)
    add_library(Python::Python UNKNOWN IMPORTED)
  endif()

  set_target_properties(Python::Python PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_SB_INC_DIR}"
  )

  if(EXISTS "${_py_rel_lib}")
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_IMPLIB_RELEASE "${_py_rel_lib}"
    )
  endif()

  if(EXISTS "${_py_dbg_lib}")
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_IMPLIB_DEBUG "${_py_dbg_lib}"
    )
  endif()

  # Fail only if absolutely nothing was found and no prior package existed
  if(NOT (EXISTS "${_py_rel_lib}" OR EXISTS "${_py_dbg_lib}" OR TARGET Python3::Python))
    message(FATAL_ERROR
      "Python import libraries not found for Windows/MSVC.\n"
      "Searched:\n  _SB_LIB_DIR='${_SB_LIB_DIR}'\n"
      "Expected names: ${_PY_RELEASE_LIB_NAME} / ${_PY_DEBUG_LIB_NAME}\n"
      "Or explicit: PYTHON_LIBRARY_RELEASE='${PYTHON_LIBRARY_RELEASE}', PYTHON_LIBRARY_DEBUG='${PYTHON_LIBRARY_DEBUG}'\n"
      "Include dir: _SB_INC_DIR='${_SB_INC_DIR}'"
    )
  endif()

  set(_SCIRUN_API_PY_OK TRUE)
endif()

if(NOT _SCIRUN_API_PY_OK)
  message(FATAL_ERROR "Python (Interpreter + Development) not found; cannot build SCIRunPythonAPI.")
endif()

if(MSVC)
  target_compile_definitions(SCIRunPythonAPI PRIVATE BOOST_PYTHON_STATIC_LIB)
endif()

target_compile_definitions(SCIRunPythonAPI PRIVATE BUILD_WITH_PYTHON)

# ====================================================================================
# Link dependencies (Python::Python now guaranteed to point to superbuild libs)
# ====================================================================================
target_link_libraries(SCIRunPythonAPI
  PRIVATE
    Core_Thread
    Core_Datatypes_Legacy_Base
    Core_Persistent
    Python::Python
    ${SCI_BOOST_LIBRARY}
)

# ====================================================================================
# Shared build definitions & Windows .pyd properties (kept from your original)
# ====================================================================================

if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_SCIRunPythonAPI)

  if(WIN32)
    # Build as a Python extension on Windows: .pyd suffix/prefixes
    set_target_properties(SCIRunPythonAPI
      PROPERTIES
        IMPORT_SUFFIX ".pyd"
        IMPORT_PREFIX ""
        PREFIX ""
        SUFFIX ".pyd"
    )
  endif()
endif()

# ====================================================================================
# Tests
# ====================================================================================

SCIRUN_ADD_TEST_DIR(Tests)

# ====================================================================================
# Diagnostics (optional but helpful)
# ====================================================================================

message(STATUS "[SCIRunPythonAPI] Python executable : ${Python_EXECUTABLE}")
message(STATUS "[SCIRunPythonAPI] Python include(s) : ${Python_INCLUDE_DIRS}")
message(STATUS "[SCIRunPythonAPI] Python libraries  : ${Python_LIBRARIES}")
