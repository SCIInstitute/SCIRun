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


# =========================
# Modules/Python CMake file
# =========================

# --- Sources & headers ---
set(Modules_Python_SRCS
  PythonObjectForwarder.cc
  InterfaceWithPython.cc
  PythonInterfaceParser.cc
  ModuleStateModifierTester.cc
  LoopEnd.cc
  LoopStart.cc
  #LoopIncrement.cc
)

set(Modules_Python_HEADERS
  PythonObjectForwarder.h
  InterfaceWithPython.h
  PythonInterfaceParser.h
  ModuleStateModifierTester.h
  LoopEnd.h
  LoopStart.h
  #LoopIncrement.h
  share.h
)

# --- Library target ---
SCIRUN_ADD_LIBRARY(Modules_Python
  ${Modules_Python_HEADERS}
  ${Modules_Python_SRCS}
)

if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Modules_Python)
endif()

# ====================================================================================
# Optional Python detection (only if the build is configured with Python support)
#   - We *always* link to Python::Python (imported target)
#   - On MSVC we override Python::Python to the superbuild import libs using IMPORTED_IMPLIB_*
# ====================================================================================
set(_SCIRUN_MODULES_PY_OK FALSE)

if(BUILD_WITH_PYTHON)
  # Prefer modern FindPython; harmless if we will override on MSVC
  find_package(Python QUIET COMPONENTS Interpreter Development)
  if(Python_Interpreter_FOUND AND Python_Development_FOUND)
    set(_SCIRUN_MODULES_PY_OK TRUE)
  else()
    find_package(Python3 QUIET COMPONENTS Interpreter Development)
    if(Python3_Interpreter_FOUND AND Python3_Development_FOUND)
      if(TARGET Python3::Python AND NOT TARGET Python::Python)
        add_library(Python::Python ALIAS Python3::Python)
      endif()
      set(_SCIRUN_MODULES_PY_OK TRUE)
    endif()
  endif()

  # --- Superbuild override on Windows/MSVC: force per-config import libs -----------
  # Superbuild should pass either:
  #   - New style:  PY_EXT_LIB_DIR (..../PCbuild/amd64), PY_INCLUDE_DIR (..../Include)
  #   - Legacy:     PYTHON_LIBRARY_DEBUG/RELEASE, PYTHON_INCLUDE_DIR
  if(MSVC)
    # Default CPython 3.13 names; adjust if you change Python version
    set(_PY_DEBUG_LIB_NAME   "python313_d.lib")
    set(_PY_RELEASE_LIB_NAME "python313.lib")

    set(_SB_LIB_DIR "${PY_EXT_LIB_DIR}")
    set(_SB_INC_DIR "${PY_INCLUDE_DIR}")

    # Fallback from legacy variables
    if(NOT _SB_LIB_DIR)
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
        list(GET Python_INCLUDE_DIRS 0 _SB_INC_DIR)
      endif()
    endif()

    # Last-resort defaults relative to inner build
    if(NOT _SB_LIB_DIR)
      set(_SB_LIB_DIR "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external/PCbuild/amd64")
    endif()
    if(NOT _SB_INC_DIR)
      set(_SB_INC_DIR "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external/Include")
    endif()

    # Resolve per-config import libraries
    set(_py_dbg_lib "")
    set(_py_rel_lib "")

    # Use explicit file paths if provided
    if(PYTHON_LIBRARY_DEBUG AND EXISTS "${PYTHON_LIBRARY_DEBUG}")
      set(_py_dbg_lib "${PYTHON_LIBRARY_DEBUG}")
    endif()
    if(PYTHON_LIBRARY_RELEASE AND EXISTS "${PYTHON_LIBRARY_RELEASE}")
      set(_py_rel_lib "${PYTHON_LIBRARY_RELEASE}")
    endif()

    # Otherwise, construct by expected names under the lib dir
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

    # Create or update Python::Python so it always points to superbuild import libs
    if(NOT TARGET Python::Python)
      add_library(Python::Python UNKNOWN IMPORTED)
    endif()

    set_target_properties(Python::Python PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${_SB_INC_DIR}"
    )

    # IMPORTANT: on MSVC the import library must be set via IMPORTED_IMPLIB_* (not IMPORTED_LOCATION_*)
    if(EXISTS "${_py_rel_lib}")
      set_target_properties(Python::Python PROPERTIES
        IMPORTED_IMPLIB_RELEASE        "${_py_rel_lib}"
        IMPORTED_IMPLIB_RELWITHDEBINFO "${_py_rel_lib}"
        IMPORTED_IMPLIB_MINSIZEREL     "${_py_rel_lib}"
      )
    endif()
    if(EXISTS "${_py_dbg_lib}")
      set_target_properties(Python::Python PROPERTIES
        IMPORTED_IMPLIB_DEBUG          "${_py_dbg_lib}"
      )
    endif()

    if(NOT (EXISTS "${_py_rel_lib}" OR EXISTS "${_py_dbg_lib}" OR TARGET Python3::Python))
      message(FATAL_ERROR
        "Python import libraries not found for Windows/MSVC.\n"
        "Searched:\n  _SB_LIB_DIR='${_SB_LIB_DIR}'\n"
        "Expected: ${_PY_RELEASE_LIB_NAME} / ${_PY_DEBUG_LIB_NAME}\n"
        "Or explicit: PYTHON_LIBRARY_RELEASE='${PYTHON_LIBRARY_RELEASE}', PYTHON_LIBRARY_DEBUG='${PYTHON_LIBRARY_DEBUG}'\n"
        "Include dir: _SB_INC_DIR='${_SB_INC_DIR}'")
    endif()

    set(_SCIRUN_MODULES_PY_OK TRUE)
  endif() # MSVC

  if(NOT _SCIRUN_MODULES_PY_OK)
    message(FATAL_ERROR "BUILD_WITH_PYTHON is ON, but Python (Interpreter + Development) could not be found.")
  endif()
endif() # BUILD_WITH_PYTHON

# ====================================================================================
# Link dependencies
# ====================================================================================

target_link_libraries(Modules_Python
  PRIVATE
    Dataflow_Network
    Core_Datatypes
    Algorithms_Base
    Core_Thread
)

if(BUILD_WITH_PYTHON)
  target_link_libraries(Modules_Python
    PRIVATE
      Python::Python
      Core_Python
  )
  target_compile_definitions(Modules_Python PRIVATE BUILD_WITH_PYTHON)
  if(MSVC)
    # If any transitive lib autolinks release python313.lib, ignore it in Debug
    target_link_options(Modules_Python PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:python313.lib>
    )
    # Optional: define if you rely on static Boost.Python in this module
    # target_compile_definitions(Modules_Python PRIVATE BOOST_PYTHON_STATIC_LIB)
  endif()
endif()

# ====================================================================================
# Tests (kept)
# ====================================================================================
if(BUILD_WITH_PYTHON)
  SCIRUN_ADD_TEST_DIR(Tests)
endif()

# ====================================================================================
# Diagnostics (show what the imported target is *actually* using)
# ====================================================================================
if(BUILD_WITH_PYTHON)
  get_target_property(_py_inc  Python::Python INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(_py_dbg  Python::Python IMPORTED_IMPLIB_DEBUG)
  get_target_property(_py_rel  Python::Python IMPORTED_IMPLIB_RELEASE)
  message(STATUS "[Modules/Python] Python include(s) : ${_py_inc}")
  message(STATUS "[Modules/Python] Python importlib  : Debug='${_py_dbg}'  Release='${_py_rel}'")
endif()