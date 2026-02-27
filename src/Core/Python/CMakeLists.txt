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
cmake_minimum_required(VERSION 3.16)

# Silence CMP0111 locally: we intentionally do NOT set IMPORTED_LOCATION_* on MSVC
# to avoid the DLL being placed on the link line by CMake/VS generators.
if(POLICY CMP0111)
  cmake_policy(SET CMP0111 OLD)
endif()

# --- Guard against system Python being injected via ${Python_LIBRARIES} / ${PYTHON_LIBRARIES}
unset(Python_LIBRARIES)
unset(PYTHON_LIBRARIES)
unset(Python_LIBRARIES CACHE)
unset(PYTHON_LIBRARIES CACHE)

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

# --- Neutralize any directory-scoped link_libraries() *before* the target is created ---
get_property(_dir_links_before DIRECTORY PROPERTY LINK_LIBRARIES)
message(STATUS "[Diag/Core_Python] DIR LINK_LIBRARIES(before)=${_dir_links_before}")

# Clear directory-scoped link libraries for THIS directory (prevents inheriting system Python)
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY LINK_LIBRARIES "")

get_property(_dir_links_after DIRECTORY PROPERTY LINK_LIBRARIES)
message(STATUS "[Diag/Core_Python] DIR LINK_LIBRARIES(after)=${_dir_links_after}")

# ---- Library target (uses project macro from top-level) ----
SCIRUN_ADD_LIBRARY(Core_Python
  ${Core_Python_HEADERS}
  ${Core_Python_SRCS}
)

# (Optional) Belt-and-suspenders wipe in case macro captured anything already
set_target_properties(Core_Python PROPERTIES LINK_LIBRARIES "")

# Preserve original shared define behavior
if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Core_Python)
endif()

# If Python enabled, expose BUILD_WITH_PYTHON so dependents can #ifdef
if(BUILD_WITH_PYTHON)
  target_compile_definitions(Core_Python PUBLIC BUILD_WITH_PYTHON)
else()
  message(STATUS "BUILD_WITH_PYTHON=OFF, Core_Python will be built in no-Python mode (code must #ifdef accordingly).")
endif()

# --- Unit tests ---
SCIRUN_ADD_TEST_DIR(Tests)

# ====================================================================================
# Python wiring (superbuild-first, robust fallbacks)
# Expected superbuild variables (preferred):
#   PYTHON_INCLUDE_DIR        -> .../Python_external/Include
#   PYTHON_PC_INCLUDE_DIR     -> .../Python_external/PC
#   PYTHON_LIBRARY_RELEASE    -> .../PCbuild/amd64/python313.lib
#   PYTHON_LIBRARY_DEBUG      -> .../PCbuild/amd64/python313_d.lib
#   PYTHON_RUNTIME_DIR        -> .../PCbuild/amd64 (optional)
#   PYTHON_EXECUTABLE         -> .../PCbuild/amd64/python.exe (optional)
# ====================================================================================

set(_SCIRUN_PY_OK FALSE)
set(_PY_INCLUDE_DIRS "")
set(_PY_EXECUTABLE "")
set(_PY_LIB_DBG "")
set(_PY_LIB_REL "")

# ---- Path-first: use explicit superbuild variables if provided ----
# Accept include dir + at least one of debug/release libs.
if(PYTHON_INCLUDE_DIR AND (PYTHON_LIBRARY_DEBUG OR PYTHON_LIBRARY_RELEASE))
  # Build a proper list of checks; skip empties so foreach doesn't see blank items.
  set(_py_checks)
  if(PYTHON_LIBRARY_DEBUG)
    list(APPEND _py_checks "${PYTHON_LIBRARY_DEBUG}")
  endif()
  if(PYTHON_LIBRARY_RELEASE)
    list(APPEND _py_checks "${PYTHON_LIBRARY_RELEASE}")
  endif()
  list(APPEND _py_checks "${PYTHON_INCLUDE_DIR}")

  foreach(_check IN LISTS _py_checks)
    if(_check AND NOT EXISTS "${_check}")
      message(FATAL_ERROR "Python superbuild hint does not exist: ${_check}")
    endif()
  endforeach()

  # Remember which ones we actually have
  set(_PY_LIB_DBG "${PYTHON_LIBRARY_DEBUG}")
  set(_PY_LIB_REL "${PYTHON_LIBRARY_RELEASE}")

  # Includes: Include (always), PC (optional on Windows for pyconfig.h)
  set(_PY_INCLUDE_DIRS "${PYTHON_INCLUDE_DIR}")
  if(PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDE_DIRS "${PYTHON_PC_INCLUDE_DIR}")
  endif()

  set(_SCIRUN_PY_OK TRUE)
endif()

# ---- Fallback: try modern FindPython / FindPython3 (hinted to superbuild root if provided) ----
if(NOT _SCIRUN_PY_OK)
  if(DEFINED PYTHON_ROOT_DIR AND EXISTS "${PYTHON_ROOT_DIR}")
    set(_PY_HINTS ROOT_DIR "${PYTHON_ROOT_DIR}")
  elseif(DEFINED PYTHON_HOME_DIR AND EXISTS "${PYTHON_HOME_DIR}")
    set(_PY_HINTS ROOT_DIR "${PYTHON_HOME_DIR}")
  else()
    set(_PY_HINTS)
  endif()

  find_package(Python QUIET COMPONENTS Interpreter Development ${_PY_HINTS})
  if(Python_Interpreter_FOUND AND Python_Development_FOUND AND TARGET Python::Python)
    set(_SCIRUN_PY_OK TRUE)
    set(_PY_EXECUTABLE "${Python_EXECUTABLE}")
    set(_PY_INCLUDE_DIRS "${Python_INCLUDE_DIRS}")
  endif()
endif()

# ---- Legacy fallback (really old CMake) ----
if(NOT _SCIRUN_PY_OK)
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)
  set(_SCIRUN_PY_OK TRUE)
  set(_PY_EXECUTABLE "${PYTHON_EXECUTABLE}")
  set(_PY_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
  # Legacy variables (PYTHON_LIBRARIES) exist, but we will still build our own imported target below.
  if(PYTHON_DEBUG_LIBRARY AND EXISTS "${PYTHON_DEBUG_LIBRARY}")
    set(_PY_LIB_DBG "${PYTHON_DEBUG_LIBRARY}")
  endif()
  if(PYTHON_LIBRARY AND EXISTS "${PYTHON_LIBRARY}")
    set(_PY_LIB_REL "${PYTHON_LIBRARY}")
  endif()
endif()

if(NOT _SCIRUN_PY_OK)
  message(FATAL_ERROR "Python (Interpreter + Development) not found; cannot build Core_Python.")
endif()

# ====================================================================================
# Ensure Python::Python imported target exists and is wired for all MSVC configs
# ====================================================================================

# Create/override the imported target
if(NOT TARGET Python::Python)
  add_library(Python::Python UNKNOWN IMPORTED)
endif()

# Prefer explicit include dir hints from superbuild for MSVC usage requirements
if(_SB_INC_DIR)
  set_target_properties(Python::Python PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_SB_INC_DIR}"
  )
elseif(_PY_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES _PY_INCLUDE_DIRS)
  set_target_properties(Python::Python PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_PY_INCLUDE_DIRS}"
  )
endif()

# ---- MSVC: set config-specific IMPORTED_IMPLIB_* (do NOT guard with EXISTS) ----
if(MSVC)
  # Make sure all common configs are recognized
  set_target_properties(Python::Python PROPERTIES
    IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
  )

  # Primary assignment (no EXISTS guards; allow the files to appear by build time)
  if(_PY_LIB_REL)
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_IMPLIB_RELEASE        "${_PY_LIB_REL}"
      IMPORTED_IMPLIB_RELWITHDEBINFO "${_PY_LIB_REL}"
      IMPORTED_IMPLIB_MINSIZEREL     "${_PY_LIB_REL}"
    )
  endif()
  if(_PY_LIB_DBG)
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_IMPLIB_DEBUG          "${_PY_LIB_DBG}"
    )
  endif()

  # Fallbacks: if one variant is missing, map it to the other so VS never sees NOTFOUND
  if(NOT _PY_LIB_DBG AND _PY_LIB_REL)
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_IMPLIB_DEBUG          "${_PY_LIB_REL}"
      MAP_IMPORTED_CONFIG_DEBUG      "Release;RelWithDebInfo"
    )
  endif()
  if(NOT _PY_LIB_REL AND _PY_LIB_DBG)
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_IMPLIB_RELEASE        "${_PY_LIB_DBG}"
      IMPORTED_IMPLIB_RELWITHDEBINFO "${_PY_LIB_DBG}"
      IMPORTED_IMPLIB_MINSIZEREL     "${_PY_LIB_DBG}"
      MAP_IMPORTED_CONFIG_RELEASE    "Debug"
      MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
      MAP_IMPORTED_CONFIG_MINSIZEREL "Debug"
    )
  endif()

else()
  # Non-Windows: point to the real binary (shared/static); skip EXISTS guards
  if(_PY_LIB_REL)
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_LOCATION_RELEASE        "${_PY_LIB_REL}"
      IMPORTED_LOCATION_RELWITHDEBINFO "${_PY_LIB_REL}"
      IMPORTED_LOCATION_MINSIZEREL     "${_PY_LIB_REL}"
    )
  endif()
  if(_PY_LIB_DBG)
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_LOCATION_DEBUG          "${_PY_LIB_DBG}"
    )
  endif()
endif()

# Helpful diagnostics (ensure we actually set something)
message(STATUS "[Diag/Core_Python] _PY_LIB_DBG='${_PY_LIB_DBG}' _PY_LIB_REL='${_PY_LIB_REL}'")
get_target_property(_py_implib_dbg Python::Python IMPORTED_IMPLIB_DEBUG)
get_target_property(_py_implib_rel Python::Python IMPORTED_IMPLIB_RELEASE)
message(STATUS "[Diag/Core_Python] PY IMPORTED_IMPLIB_DEBUG='${_py_implib_dbg}'")
message(STATUS "[Diag/Core_Python] PY IMPORTED_IMPLIB_RELEASE='${_py_implib_rel}'")

# ====================================================================================
# Platform-specific compile definitions (PYTHONPATH, etc.)
# ====================================================================================

# Compute python name like python313 for macros/diagnostics
set(_PYTHON_NAME "")
set(_SCIRUN_PY_DIGITS "")  # "313" form

if(NOT _SCIRUN_PY_DIGITS)
  foreach(_cand "${_PY_LIB_DBG}" "${_PY_LIB_REL}")
    if(_cand)
      get_filename_component(_cand_name "${_cand}" NAME)
      if(_cand_name MATCHES "python([0-9]+)(_d)?\\.lib$")
        set(_SCIRUN_PY_DIGITS "${CMAKE_MATCH_1}")
        break()
      endif()
    endif()
  endforeach()
endif()
if(NOT _SCIRUN_PY_DIGITS AND DEFINED Python_VERSION_MAJOR AND DEFINED Python_VERSION_MINOR)
  string(CONCAT _SCIRUN_PY_DIGITS "${Python_VERSION_MAJOR}" "${Python_VERSION_MINOR}")
endif()
if(_SCIRUN_PY_DIGITS)
  set(_PYTHON_NAME "python${_SCIRUN_PY_DIGITS}")
endif()

# Try to compute parent of site-packages using Python (optional)
set(_PYTHON_MODULE_SEARCH_PARENT "")
if(PYTHON_EXECUTABLE AND EXISTS "${PYTHON_EXECUTABLE}")
  set(_SCIRUN_PY_QUERY
    "import sysconfig; p=sysconfig.get_paths(); print((p.get('purelib') or p.get('platlib') or ''))"
  )
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "${_SCIRUN_PY_QUERY}"
    OUTPUT_VARIABLE _PY_SITE
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(_PY_SITE)
    get_filename_component(_PYTHON_MODULE_SEARCH_PARENT "${_PY_SITE}" DIRECTORY)
    file(TO_CMAKE_PATH "${_PYTHON_MODULE_SEARCH_PARENT}" _PYTHON_MODULE_SEARCH_PARENT)
  endif()
endif()

# Apply includes + macros
target_include_directories(Core_Python PRIVATE ${_PY_INCLUDE_DIRS})
if(WIN32)
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
    "PYTHONNAME=L\"${_PYTHON_NAME}\""
  )
elseif(APPLE)
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
    "PYTHONLIBDIR=L\"\""
    "PYTHONLIB=L\"\""
  )
else()
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
  )
endif()

# ====================================================================================
# Link dependencies (all PRIVATE to avoid leaking third-party includes to dependents)
# ====================================================================================
target_link_libraries(Core_Python
  PRIVATE
    Python::Python
    SCIRunPythonAPI
    ${SCI_BOOST_LIBRARY}
    Core_Matlab
    Core_Datatypes
    Core_Datatypes_Legacy_Field
)

# --- Debug-only: ignore autolinked *release* python import lib (e.g., from Boost.Python) ---
if(MSVC)
  if(_SCIRUN_PY_DIGITS)
    set(_SCIRUN_PY_RELEASE_NAME "python${_SCIRUN_PY_DIGITS}.lib")
    target_link_options(Core_Python PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:${_SCIRUN_PY_RELEASE_NAME}>
    )
  else()
    target_link_options(Core_Python PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:python3.lib>
    )
  endif()
endif()

# ====================================================================================
# Boost.Python handling
# ====================================================================================
option(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK "Explicitly link Boost.Python on MSVC; otherwise rely on autolink" OFF)
set(_SCIRUN_PY_M "${_SCIRUN_PY_DIGITS}")

if(MSVC)
  if(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK)
    target_compile_definitions(Core_Python PRIVATE BOOST_PYTHON_NO_LIB BOOST_PYTHON_STATIC_LIB)
    if (NOT DEFINED SCI_BOOST_LIBRARY_DIR OR NOT EXISTS "${SCI_BOOST_LIBRARY_DIR}")
      message(FATAL_ERROR "SCI_BOOST_LIBRARY_DIR is not set or does not exist. Pass it from the superbuild.")
    endif()
    file(GLOB _bp_dbg_candidates
      "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib"
      "${SCI_BOOST_LIBRARY_DIR}/boost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib"
    )
    file(GLOB _bp_rel_candidates_all
      "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-*.lib"
      "${SCI_BOOST_LIBRARY_DIR}/boost_python${_SCIRUN_PY_M}-vc*-mt-*.lib"
    )
    set(_bp_rel_candidates "")
    foreach(_cand IN LISTS _bp_rel_candidates_all)
      if(NOT _cand MATCHES "-gd-")
        list(APPEND _bp_rel_candidates "${_cand}")
      endif()
    endforeach()
    if(_bp_dbg_candidates)
      list(SORT _bp_dbg_candidates)
      list(REVERSE _bp_dbg_candidates)
      list(GET _bp_dbg_candidates 0 SCI_BOOST_PYTHON_LIB_DEBUG)
    endif()
    if(_bp_rel_candidates)
      list(SORT _bp_rel_candidates)
      list(REVERSE _bp_rel_candidates)
      list(GET _bp_rel_candidates 0 SCI_BOOST_PYTHON_LIB_RELEASE)
    endif()
    if(NOT EXISTS "${SCI_BOOST_PYTHON_LIB_DEBUG}" OR NOT EXISTS "${SCI_BOOST_PYTHON_LIB_RELEASE}")
      message(FATAL_ERROR "Boost.Python libs not found under '${SCI_BOOST_LIBRARY_DIR}'.")
    endif()
    target_link_libraries(Core_Python PRIVATE
      debug     "${SCI_BOOST_PYTHON_LIB_DEBUG}"
      optimized "${SCI_BOOST_PYTHON_LIB_RELEASE}"
    )
  else()
    if (DEFINED SCI_BOOST_LIBRARY_DIR AND EXISTS "${SCI_BOOST_LIBRARY_DIR}")
      target_link_directories(Core_Python PRIVATE "${SCI_BOOST_LIBRARY_DIR}")
    endif()
    message(STATUS "[Core/Python] Using MSVC autolink for Boost.Python (ensure lib names match).")
  endif()
else()
  set(_bp_names "boost_python${_SCIRUN_PY_M}" "boost_python3" "boost_python")
  find_library(BOOST_PYTHON_LIB NAMES ${_bp_names} PATHS "${SCI_BOOST_LIBRARY_DIR}" "${SCI_BOOST_PREFIX}/lib" "${SCI_BOOST_PREFIX}/lib64" NO_DEFAULT_PATH)
  if(NOT BOOST_PYTHON_LIB)
    find_library(BOOST_PYTHON_LIB NAMES ${_bp_names})
  endif()
  if(BOOST_PYTHON_LIB)
    target_link_libraries(Core_Python PRIVATE "${BOOST_PYTHON_LIB}")
    message(STATUS "[Core/Python] Linking Boost.Python: ${BOOST_PYTHON_LIB}")
  else()
    message(WARNING "[Core/Python] Boost.Python library not found on this platform.")
  endif()
endif()

# Debug-only macro (matches your previous behavior)
set_target_properties(Core_Python PROPERTIES COMPILE_DEFINITIONS_DEBUG "BOOST_DEBUG_PYTHON")

# ---- TEMP: strip any explicit system Python importlib if it leaked in ----
get_target_property(_cp_links Core_Python LINK_LIBRARIES)
if(_cp_links)
  set(_cleaned "")
  foreach(_L IN LISTS _cp_links)
    file(TO_CMAKE_PATH "${_L}" _N)
    if(_N MATCHES "^C:/Program Files/[^/]*/Python[^/]*/libs/python[0-9]+(_d)?\\.lib$")
      # skip system python lib
    else()
      list(APPEND _cleaned "${_L}")
    endif()
  endforeach()
  set_target_properties(Core_Python PROPERTIES LINK_LIBRARIES "${_cleaned}")
endif()

# ---- Helpful diagnostics (actual evaluated properties) ----
get_target_property(_py_implib_dbg Python::Python IMPORTED_IMPLIB_DEBUG)
get_target_property(_py_implib_rel Python::Python IMPORTED_IMPLIB_RELEASE)
get_target_property(_cp_links2     Core_Python     LINK_LIBRARIES)

message(STATUS "[Diag/Core_Python] PY IMPORTED_IMPLIB_DEBUG='${_py_implib_dbg}'")
message(STATUS "[Diag/Core_Python] PY IMPORTED_IMPLIB_RELEASE='${_py_implib_rel}'")
message(STATUS "[Diag/Core_Python] LINK_LIBRARIES=${_cp_links2}")