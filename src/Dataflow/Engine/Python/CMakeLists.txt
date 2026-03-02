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

cmake_minimum_required(VERSION 3.16)

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
# Ensure an alias imported target 'SCIRunPython' exists and points to the CPython
# import libraries (Windows/MSVC) or the shared/static python lib on other platforms.
# Prefer the alias from Core/Python; if missing, create a compatible one here.
# ====================================================================================

set(_PY_INCLUDE_DIRS "")
set(_PY_LIB_REL "")
set(_PY_LIB_DBG "")

# Prefer superbuild hints (typical when using PCbuild layout on Windows)
if(PYTHON_LIBRARY_RELEASE)
  set(_PY_LIB_REL "${PYTHON_LIBRARY_RELEASE}")
endif()
if(PYTHON_LIBRARY_DEBUG)
  set(_PY_LIB_DBG "${PYTHON_LIBRARY_DEBUG}")
endif()

# Also accept short names (if provided)
if(NOT _PY_LIB_REL AND DEFINED PY_EXT_LIB_DIR)
  if(EXISTS "${PY_EXT_LIB_DIR}/python313.lib")
    set(_PY_LIB_REL "${PY_EXT_LIB_DIR}/python313.lib")
  endif()
endif()
if(NOT _PY_LIB_DBG AND DEFINED PY_EXT_LIB_DIR)
  if(EXISTS "${PY_EXT_LIB_DIR}/python313_d.lib")
    set(_PY_LIB_DBG "${PY_EXT_LIB_DIR}/python313_d.lib")
  endif()
endif()

# Includes: prefer superbuild variables
if(PYTHON_INCLUDE_DIR)
  list(APPEND _PY_INCLUDE_DIRS "${PYTHON_INCLUDE_DIR}")
endif()
if(PY_INCLUDE_DIR)
  list(APPEND _PY_INCLUDE_DIRS "${PY_INCLUDE_DIR}")
endif()
if(PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
  list(APPEND _PY_INCLUDE_DIRS "${PYTHON_PC_INCLUDE_DIR}")
endif()

# If still incomplete, try modern FindPython to harvest includes/libs
if((NOT _PY_INCLUDE_DIRS) OR (NOT _PY_LIB_REL AND NOT _PY_LIB_DBG))
  find_package(Python QUIET COMPONENTS Interpreter Development)
  if(Python_Development_FOUND AND TARGET Python::Python)
    if(NOT _PY_INCLUDE_DIRS AND Python_INCLUDE_DIRS)
      set(_PY_INCLUDE_DIRS "${Python_INCLUDE_DIRS}")
    endif()
    # Harvest import libs if present (MSVC)
    get_target_property(_try_rel Python::Python IMPORTED_IMPLIB_RELEASE)
    get_target_property(_try_dbg Python::Python IMPORTED_IMPLIB_DEBUG)
    if(NOT _PY_LIB_REL AND _try_rel)
      set(_PY_LIB_REL "${_try_rel}")
    endif()
    if(NOT _PY_LIB_DBG AND _try_dbg)
      set(_PY_LIB_DBG "${_try_dbg}")
    endif()
    # Non-MSVC: fall back to IMPORTED_LOCATION_* (shared/static)
    if(NOT MSVC)
      if(NOT _PY_LIB_REL)
        get_target_property(_try_loc_rel Python::Python IMPORTED_LOCATION_RELEASE)
        if(_try_loc_rel)
          set(_PY_LIB_REL "${_try_loc_rel}")
        endif()
      endif()
      if(NOT _PY_LIB_DBG)
        get_target_property(_try_loc_dbg Python::Python IMPORTED_LOCATION_DEBUG)
        if(_try_loc_dbg)
          set(_PY_LIB_DBG "${_try_loc_dbg}")
        endif()
      endif()
    endif()
  endif()
endif()

# If Core/Python has already created SCIRunPython, reuse it.
if(NOT TARGET SCIRunPython)
  # Create a compatible alias imported target locally
  add_library(SCIRunPython STATIC IMPORTED GLOBAL)

  if(_PY_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES _PY_INCLUDE_DIRS)
    set_target_properties(SCIRunPython PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${_PY_INCLUDE_DIRS}"
    )
  endif()

  if(MSVC)
    set_target_properties(SCIRunPython PROPERTIES
      IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
    )
    if(_PY_LIB_REL)
      set_target_properties(SCIRunPython PROPERTIES
        IMPORTED_LOCATION_RELEASE        "${_PY_LIB_REL}"
        IMPORTED_LOCATION_RELWITHDEBINFO "${_PY_LIB_REL}"
        IMPORTED_LOCATION_MINSIZEREL     "${_PY_LIB_REL}"
      )
    endif()
    if(_PY_LIB_DBG)
      set_target_properties(SCIRunPython PROPERTIES
        IMPORTED_LOCATION_DEBUG          "${_PY_LIB_DBG}"
      )
    endif()
    # Map missing variants so the generator never sees NOTFOUND
    if(NOT _PY_LIB_DBG AND _PY_LIB_REL)
      set_target_properties(SCIRunPython PROPERTIES
        IMPORTED_LOCATION_DEBUG          "${_PY_LIB_REL}"
        MAP_IMPORTED_CONFIG_DEBUG        "Release;RelWithDebInfo"
      )
    endif()
    if(NOT _PY_LIB_REL AND _PY_LIB_DBG)
      set_target_properties(SCIRunPython PROPERTIES
        IMPORTED_LOCATION_RELEASE        "${_PY_LIB_DBG}"
        IMPORTED_LOCATION_RELWITHDEBINFO "${_PY_LIB_DBG}"
        IMPORTED_LOCATION_MINSIZEREL     "${_PY_LIB_DBG}"
        MAP_IMPORTED_CONFIG_RELEASE      "Debug"
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
        MAP_IMPORTED_CONFIG_MINSIZEREL   "Debug"
      )
    endif()
  else()
    # Non-MSVC: the Python library may be a shared/static real library
    if(_PY_LIB_REL)
      set_target_properties(SCIRunPython PROPERTIES
        IMPORTED_LOCATION_RELEASE        "${_PY_LIB_REL}"
        IMPORTED_LOCATION_RELWITHDEBINFO "${_PY_LIB_REL}"
        IMPORTED_LOCATION_MINSIZEREL     "${_PY_LIB_REL}"
      )
    endif()
    if(_PY_LIB_DBG)
      set_target_properties(SCIRunPython PROPERTIES
        IMPORTED_LOCATION_DEBUG          "${_PY_LIB_DBG}"
      )
    endif()
  endif()
endif()

# ====================================================================================
# Compile definitions
# ====================================================================================
if(MSVC)
  # This target does not use Boost.Python symbols directly; static lib define is harmless.
  target_compile_definitions(SCIRunPythonAPI PRIVATE BOOST_PYTHON_STATIC_LIB)
endif()
target_compile_definitions(SCIRunPythonAPI PRIVATE BUILD_WITH_PYTHON)

# ====================================================================================
# Link dependencies (Python via SCIRunPython alias; PRIVATE to avoid leaking)
# ====================================================================================
target_link_libraries(SCIRunPythonAPI
  PRIVATE
    Core_Thread
    Core_Datatypes_Legacy_Base
    Core_Persistent
    SCIRunPython
    ${SCI_BOOST_LIBRARY}
)

# ------------------------------------------------------------------------------------
# Debug-only insurance: ignore any autolinked *release* python import lib
# (e.g., if a third-party .lib contains /DEFAULTLIB:python313.lib directives).
# ------------------------------------------------------------------------------------
# Derive python digits (e.g. "313") from the selected import libraries
set(_SCIRUN_PY_DIGITS "")
foreach(_cand "${_PY_LIB_DBG}" "${_PY_LIB_REL}")
  if(_cand)
    get_filename_component(_cand_name "${_cand}" NAME)
    if(_cand_name MATCHES "python([0-9]+)(_d)?\\.lib$")
      set(_SCIRUN_PY_DIGITS "${CMAKE_MATCH_1}")
      break()
    endif()
  endif()
endforeach()

if(MSVC)
  if(_SCIRUN_PY_DIGITS)
    target_link_options(SCIRunPythonAPI PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:python${_SCIRUN_PY_DIGITS}.lib>
    )
  else()
    target_link_options(SCIRunPythonAPI PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:python3.lib>
    )
  endif()
endif()

# ====================================================================================
# Shared build: Windows .pyd properties (kept)
# ====================================================================================
if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_SCIRunPythonAPI)
  if(WIN32)
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
# Diagnostics (show what the alias imported target is actually using)
# ====================================================================================
get_target_property(_api_py_inc SCIRunPython INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(_api_py_dbg SCIRunPython IMPORTED_LOCATION_DEBUG)
get_target_property(_api_py_rel SCIRunPython IMPORTED_LOCATION_RELEASE)
message(STATUS "[SCIRunPythonAPI] Python include(s) : ${_api_py_inc}")
message(STATUS "[SCIRunPythonAPI] Python importlib  : Debug='${_api_py_dbg}'  Release='${_api_py_rel}'")