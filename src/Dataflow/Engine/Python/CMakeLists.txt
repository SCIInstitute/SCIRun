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
# Cross-platform Python detection
#   Prefer modern FindPython / FindPython3; fall back to legacy modules if needed.
#   Result:
#     - Imported target to link: Python::Python
#     - Vars: Python_EXECUTABLE, Python_INCLUDE_DIRS, Python_LIBRARIES
# ====================================================================================

set(_SCIRUN_API_PY_OK FALSE)

# 1) Prefer the generic FindPython (Interpreter + Development)
find_package(Python QUIET COMPONENTS Interpreter Development)
if(Python_Interpreter_FOUND AND Python_Development_FOUND)
  set(_SCIRUN_API_PY_OK TRUE)
elseif(NOT _SCIRUN_API_PY_OK)
  # 2) Try Python3 module
  find_package(Python3 QUIET COMPONENTS Interpreter Development)
  if(Python3_Interpreter_FOUND AND Python3_Development_FOUND)
    # Normalize into Python_* variable names for consistency
    set(Python_EXECUTABLE   "${Python3_EXECUTABLE}")
    set(Python_INCLUDE_DIRS "${Python3_INCLUDE_DIRS}")
    set(Python_LIBRARIES    "${Python3_LIBRARIES}")
    # Alias target so we can always link Python::Python
    if(TARGET Python3::Python AND NOT TARGET Python::Python)
      add_library(Python::Python ALIAS Python3::Python)
    endif()
    set(_SCIRUN_API_PY_OK TRUE)
  endif()
endif()

# 3) Legacy fallback path (older CMake or environments without the newer modules)
if(NOT _SCIRUN_API_PY_OK)
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)
  # Normalize var names
  set(Python_EXECUTABLE   "${PYTHON_EXECUTABLE}")
  set(Python_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
  set(Python_LIBRARIES    "${PYTHON_LIBRARIES}")

  # Provide a compatible imported target so target_link_libraries can always use Python::Python
  if(NOT TARGET Python::Python)
    add_library(Python::Python UNKNOWN IMPORTED)
    set_target_properties(Python::Python PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Python_INCLUDE_DIRS}"
    )
    if(WIN32)
      # On MSVC, the import library (.lib) is what we link against
      set_target_properties(Python::Python PROPERTIES
        IMPORTED_IMPLIB "${Python_LIBRARIES}"
      )
    else()
      set_target_properties(Python::Python PROPERTIES
        IMPORTED_LOCATION "${Python_LIBRARIES}"
      )
    endif()
  endif()

  set(_SCIRUN_API_PY_OK TRUE)
endif()

if(MSVC)
  target_compile_definitions(SCIRunPythonAPI PRIVATE BOOST_PYTHON_STATIC_LIB)
endif()

if(NOT _SCIRUN_API_PY_OK)
  message(FATAL_ERROR "Python (Interpreter + Development) not found; cannot build SCIRunPythonAPI.")
endif()

# ====================================================================================
# Link dependencies
# ====================================================================================

# Replace legacy ${SCI_PYTHON_LIBRARY} with imported target Python::Python.
# This pulls the include dir for Python.h and the correct platform library.
target_link_libraries(SCIRunPythonAPI
  PRIVATE
    Core_Thread
    Core_Datatypes_Legacy_Base
    Core_Persistent
    Core_Python
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
