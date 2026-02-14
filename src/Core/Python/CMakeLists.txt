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

# --- Library target (uses project macro from top-level) ---
SCIRUN_ADD_LIBRARY(Core_Python
  ${Core_Python_HEADERS}
  ${Core_Python_SRCS}
)

if(BUILD_WITH_PYTHON)
  # PUBLIC so any target that links Core_Python also sees BUILD_WITH_PYTHON
  target_compile_definitions(Core_Python PUBLIC BUILD_WITH_PYTHON)
else()
  # Optional: prevent building this lib at all if Python is off
  message(STATUS "BUILD_WITH_PYTHON=OFF, Core_Python will not be used")
endif()

if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Core_Python)
endif()

# --- Unit tests subdir (kept as in your original) ---
SCIRUN_ADD_TEST_DIR(Tests)

# ====================================================================================
# Cross-platform Python detection (modern-first, legacy fallback for older CMake)
# ====================================================================================

# We’ll try in this order:
#   1) FindPython (CMake >= ~3.15)
#   2) FindPython3 (CMake >= ~3.12)
#   3) Legacy FindPythonInterp + FindPythonLibs (CMake >= 3.0)
#
# The result will be:
#   - Imported target to link: Python::Python
#   - Variables available: Python_EXECUTABLE, Python_INCLUDE_DIRS, Python_LIBRARIES
#
set(_SCIRUN_PY_OK FALSE)

# Prefer FindPython (generic)
find_package(Python QUIET COMPONENTS Interpreter Development)
if(Python_Interpreter_FOUND AND Python_Development_FOUND)
  set(_SCIRUN_PY_OK TRUE)
elseif(NOT _SCIRUN_PY_OK)
  # Try Python3 module
  find_package(Python3 QUIET COMPONENTS Interpreter Development)
  if(Python3_Interpreter_FOUND AND Python3_Development_FOUND)
    # Normalize variable names to Python_* so later logic is uniform
    set(Python_EXECUTABLE  "${Python3_EXECUTABLE}")
    set(Python_INCLUDE_DIRS "${Python3_INCLUDE_DIRS}")
    set(Python_LIBRARIES    "${Python3_LIBRARIES}")
    # Create a consistent alias so we can always link Python::Python
    if(TARGET Python3::Python AND NOT TARGET Python::Python)
      add_library(Python::Python ALIAS Python3::Python)
    endif()
    set(_SCIRUN_PY_OK TRUE)
  endif()
endif()

# Legacy fallback (older CMake or no module found)
if(NOT _SCIRUN_PY_OK)
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED) # populates PYTHON_INCLUDE_DIRS / PYTHON_LIBRARIES in old module
  # Normalize variable names
  set(Python_EXECUTABLE   "${PYTHON_EXECUTABLE}")
  set(Python_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
  set(Python_LIBRARIES    "${PYTHON_LIBRARIES}")
  # Provide a compatible imported target Python::Python
  if(NOT TARGET Python::Python)
    add_library(Python::Python UNKNOWN IMPORTED)
    set_target_properties(Python::Python PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Python_INCLUDE_DIRS}"
    )
    if(WIN32)
      # On Windows, the link library is an import library (.lib)
      set_target_properties(Python::Python PROPERTIES
        IMPORTED_IMPLIB "${Python_LIBRARIES}"
      )
    else()
      set_target_properties(Python::Python PROPERTIES
        IMPORTED_LOCATION "${Python_LIBRARIES}"
      )
    endif()
  endif()
  set(_SCIRUN_PY_OK TRUE)
endif()

if(NOT _SCIRUN_PY_OK)
  message(FATAL_ERROR "Python (Interpreter + Development) not found; cannot build Core_Python.")
endif()

# ====================================================================================
# Platform-specific compile definitions derived from the detected Python
# ====================================================================================

# Compute a best-effort Python module search path (a parent of site-packages)
# Ask Python directly (portable across platforms/envs).
set(_SCIRUN_PY_QUERY
  "import sys,sysconfig; p=sysconfig.get_paths(); print((p.get('purelib') or p.get('platlib') or ''))"
)
execute_process(
  COMMAND "${Python_EXECUTABLE}" -c "${_SCIRUN_PY_QUERY}"
  OUTPUT_VARIABLE _PY_SITE
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Parent of site-packages (keeps your original semantics)
set(_PYTHON_MODULE_SEARCH_PARENT "${_PY_SITE}")
if(_PYTHON_MODULE_SEARCH_PARENT)
  get_filename_component(_PYTHON_MODULE_SEARCH_PARENT "${_PY_SITE}" DIRECTORY)
endif()

# Compute a name like pythonXY (e.g., python311) if your code needs it
set(_SCIRUN_PY_NAME_QUERY "import sys; print('python{}{}'.format(sys.version_info.major, sys.version_info.minor))")
execute_process(
  COMMAND "${Python_EXECUTABLE}" -c "${_SCIRUN_PY_NAME_QUERY}"
  OUTPUT_VARIABLE _PYTHON_NAME
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# For macOS legacy defines that your code used:
set(_PYTHON_LIB "")
if(Python_LIBRARIES)
  # If multiple libs are reported, take the first
  if(Python_LIBRARIES MATCHES ";")
    list(GET Python_LIBRARIES 0 _PYTHON_LIB)
  else()
    set(_PYTHON_LIB "${Python_LIBRARIES}")
  endif()
endif()
get_filename_component(_PYTHON_LIBDIR "${_PYTHON_LIB}" DIRECTORY)

# Apply your original platform-specific -D defines, but using the computed values
if(WIN32)
  # Original: -DPYTHONPATH=L"${SCI_PYTHON_MODULE_PARENT_PATH}" -DPYTHONNAME=L"${SCI_PYTHON_NAME}"
  add_definitions(
    -DPYTHONPATH=L"${_PYTHON_MODULE_SEARCH_PARENT}"
    -DPYTHONNAME=L"${_PYTHON_NAME}"
  )
elseif(APPLE)
  # Original: also defined PYTHONLIBDIR and PYTHONLIB on macOS
  add_definitions(
    -DPYTHONPATH=L"${_PYTHON_MODULE_SEARCH_PARENT}"
    -DPYTHONLIBDIR=L"${_PYTHON_LIBDIR}"
    -DPYTHONLIB=L"${_PYTHON_LIB}"
  )
else()
  # Linux/Unix: only PYTHONPATH originally
  add_definitions(
    -DPYTHONPATH=L"${_PYTHON_MODULE_SEARCH_PARENT}"
  )
endif()

# ====================================================================================
# Link dependencies
# ====================================================================================

# Note:
#  - Python::Python carries include dirs and platform-correct libs
#  - Keep your other links intact
target_link_libraries(Core_Python
  PRIVATE
    Python::Python
    ${SCI_BOOST_LIBRARY}
    Core_Matlab
    Core_Datatypes
    Core_Datatypes_Legacy_Field
)

# Optional: a little debug output so you can verify in configure logs
message(STATUS "[Core/Python] Using Python executable: ${Python_EXECUTABLE}")
message(STATUS "[Core/Python] Using Python include(s): ${Python_INCLUDE_DIRS}")
message(STATUS "[Core/Python] Using Python library   : ${Python_LIBRARIES}")
message(STATUS "[Core/Python] PYTHONPATH define     : ${_PYTHON_MODULE_SEARCH_PARENT}")
if(APPLE)
  message(STATUS "[Core/Python] macOS PYTHONLIBDIR    : ${_PYTHON_LIBDIR}")
  message(STATUS "[Core/Python] macOS PYTHONLIB       : ${_PYTHON_LIB}")
endif()

# Keep your existing debug-only definition (unchanged)
set_target_properties(Core_Python
  PROPERTIES
    COMPILE_DEFINITIONS_DEBUG "BOOST_DEBUG_PYTHON"
)