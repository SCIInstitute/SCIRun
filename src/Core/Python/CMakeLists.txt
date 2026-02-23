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

# Library target (uses project macro from top-level)
SCIRUN_ADD_LIBRARY(Core_Python
  ${Core_Python_HEADERS}
  ${Core_Python_SRCS}
)

# --- Also wipe anything that was still captured on the target itself (belt-and-suspenders) ---
set_target_properties(Core_Python PROPERTIES LINK_LIBRARIES "")

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
# Python wiring (superbuild-first, robust fallbacks)
# Expected superbuild variables (preferred):
#   PYTHON_INCLUDE_DIR       -> .../Python_external/Include
#   PYTHON_PC_INCLUDE_DIR    -> .../Python_external/PC            (optional but recommended on Windows)
#   PYTHON_LIBRARY_RELEASE   -> .../PCbuild/amd64/python313.lib
#   PYTHON_LIBRARY_DEBUG     -> .../PCbuild/amd64/python313_d.lib
#   PYTHON_RUNTIME_DIR       -> .../PCbuild/amd64                 (optional, for DLLs at runtime)
#   PYTHON_EXECUTABLE        -> .../PCbuild/amd64/python.exe      (optional, used for PYTHONPATH query)
# ====================================================================================

set(_SCIRUN_PY_OK FALSE)
set(_PY_INCLUDE_DIRS "")
set(_PY_EXECUTABLE "")
set(_PY_LIB_DBG "")
set(_PY_LIB_REL "")

# ---- Path-first: use explicit superbuild variables if provided ----
# Expected (from superbuild):
#   PYTHON_INCLUDE_DIR       -> .../Python_external/Include
#   PYTHON_PC_INCLUDE_DIR    -> .../Python_external/PC
#   PYTHON_LIBRARY_RELEASE   -> .../PCbuild/amd64/python313.lib
#   PYTHON_LIBRARY_DEBUG     -> .../PCbuild/amd64/python313_d.lib
#   PYTHON_RUNTIME_DIR       -> .../PCbuild/amd64            (optional)
#   PYTHON_EXECUTABLE        -> .../PCbuild/amd64/python.exe (optional)

if(PYTHON_LIBRARY_DEBUG AND PYTHON_LIBRARY_RELEASE AND PYTHON_INCLUDE_DIR)
  if(NOT EXISTS "${PYTHON_LIBRARY_DEBUG}")
    message(FATAL_ERROR "PYTHON_LIBRARY_DEBUG does not exist: ${PYTHON_LIBRARY_DEBUG}")
  endif()
  if(NOT EXISTS "${PYTHON_LIBRARY_RELEASE}")
    message(FATAL_ERROR "PYTHON_LIBRARY_RELEASE does not exist: ${PYTHON_LIBRARY_RELEASE}")
  endif()
  if(NOT EXISTS "${PYTHON_INCLUDE_DIR}")
    message(FATAL_ERROR "PYTHON_INCLUDE_DIR does not exist: ${PYTHON_INCLUDE_DIR}")
  endif()

  set(_PY_LIB_DBG "${PYTHON_LIBRARY_DEBUG}")
  set(_PY_LIB_REL "${PYTHON_LIBRARY_RELEASE}")

  # Includes: Include (always), PC (optional on Windows for pyconfig.h)
  set(_PY_INCLUDE_DIRS "${PYTHON_INCLUDE_DIR}")
  if(PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDE_DIRS "${PYTHON_PC_INCLUDE_DIR}")
  endif()

  # Create or override the imported target to **force** superbuild Python
  if(NOT TARGET Python::Python)
    add_library(Python::Python UNKNOWN IMPORTED)
  endif()

  if(MSVC)
    # On MSVC, link against the import library via IMPORTED_IMPLIB_* (NOT IMPORTED_LOCATION_*)
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_IMPLIB_DEBUG         "${_PY_LIB_DBG}"
      IMPORTED_IMPLIB_RELWITHDEBINFO "${_PY_LIB_REL}"
      IMPORTED_IMPLIB_RELEASE       "${_PY_LIB_REL}"
      IMPORTED_IMPLIB_MINSIZEREL    "${_PY_LIB_REL}"
      INTERFACE_INCLUDE_DIRECTORIES "${_PY_INCLUDE_DIRS}"
    )
  else()
    # Non-Windows: link directly to the shared/static lib path
    set_target_properties(Python::Python PROPERTIES
      IMPORTED_LOCATION_DEBUG         "${_PY_LIB_DBG}"
      IMPORTED_LOCATION_RELWITHDEBINFO "${_PY_LIB_REL}"
      IMPORTED_LOCATION_RELEASE       "${_PY_LIB_REL}"
      IMPORTED_LOCATION_MINSIZEREL    "${_PY_LIB_REL}"
      INTERFACE_INCLUDE_DIRECTORIES   "${_PY_INCLUDE_DIRS}"
    )
  endif()

  # Provide a best-effort Python_EXECUTABLE for diagnostics/macros
  if(PYTHON_EXECUTABLE AND EXISTS "${PYTHON_EXECUTABLE}")
    set(_PY_EXECUTABLE "${PYTHON_EXECUTABLE}")
  elseif(PYTHON_RUNTIME_DIR AND EXISTS "${PYTHON_RUNTIME_DIR}")
    if(EXISTS "${PYTHON_RUNTIME_DIR}/python.exe")
      set(_PY_EXECUTABLE "${PYTHON_RUNTIME_DIR}/python.exe")
    endif()
  endif()

  set(_SCIRUN_PY_OK TRUE)
endif()

# ---- Fallback: try modern FindPython / FindPython3 (scoped to superbuild root if hinted) ----
if(NOT _SCIRUN_PY_OK)
  # If the superbuild gave us a root, hint to CMake
  if(DEFINED PYTHON_ROOT_DIR AND EXISTS "${PYTHON_ROOT_DIR}")
    set(_PY_HINTS ROOT_DIR "${PYTHON_ROOT_DIR}")
  elseif(DEFINED PYTHON_HOME_DIR AND EXISTS "${PYTHON_HOME_DIR}")
    set(_PY_HINTS ROOT_DIR "${PYTHON_HOME_DIR}")
  else()
    set(_PY_HINTS)
  endif()

  # Prefer modern module name "Python" (CMake >= 3.15)
  find_package(Python QUIET COMPONENTS Interpreter Development ${_PY_HINTS})
  if(Python_Interpreter_FOUND AND Python_Development_FOUND)
    if(TARGET Python::Python)
      # Good: prefer the imported target directly
      set(_SCIRUN_PY_OK TRUE)
      set(_PY_EXECUTABLE "${Python_EXECUTABLE}")
      # Extract library paths for later macros, if available
      set(_PY_INCLUDE_DIRS "${Python_INCLUDE_DIRS}")
      # Try to collect per-config (if provided)
      # Python::Python usually has single-location; but it's fine for MSVC if it resolves to .lib
    endif()
  endif()
endif()

# ---- Legacy fallback (really old CMake) ----
if(NOT _SCIRUN_PY_OK)
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)
  if(NOT TARGET Python::Python)
    add_library(Python::Python UNKNOWN IMPORTED)
    set_target_properties(Python::Python PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${PYTHON_INCLUDE_DIRS}"
    )
    if(WIN32)
      set_target_properties(Python::Python PROPERTIES IMPORTED_IMPLIB "${PYTHON_LIBRARIES}")
    else()
      set_target_properties(Python::Python PROPERTIES IMPORTED_LOCATION "${PYTHON_LIBRARIES}")
    endif()
  endif()
  set(_SCIRUN_PY_OK TRUE)
  set(_PY_EXECUTABLE "${PYTHON_EXECUTABLE}")
  set(_PY_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
endif()

if(NOT _SCIRUN_PY_OK)
  message(FATAL_ERROR "Python (Interpreter + Development) not found; cannot build Core_Python.")
endif()

# ====================================================================================
# Platform-specific compile definitions (PYTHONPATH, etc.)
# - We always define the macros so sources never see an undefined identifier.
# - If we have a Python executable, we query sysconfig for site-packages parent; otherwise fallback to empty.
# ====================================================================================

# Compute python name like python313 for macros/diagnostics
set(_PYTHON_NAME "")
set(_SCIRUN_PY_DIGITS "")  # "313" form; useful for Boost.Python lib name

# Try derive digits from explicit libraries first (handles 3.10+)
if(NOT _SCIRUN_PY_DIGITS)
  foreach(_cand "${_PY_LIB_DBG}" "${_PY_LIB_REL}")
    if(_cand)
      get_filename_component(_cand_name "${_cand}" NAME)
      # Match python313[_d].lib or libpython313[_d].lib
      if(_cand_name MATCHES "python([0-9]+)(_d)?\\.lib$")
        set(_SCIRUN_PY_DIGITS "${CMAKE_MATCH_1}")
        break()
      endif()
    endif()
  endforeach()
endif()

# If CMake provided version numbers, build name from those
if(NOT _SCIRUN_PY_DIGITS AND DEFINED Python_VERSION_MAJOR AND DEFINED Python_VERSION_MINOR)
  string(CONCAT _SCIRUN_PY_DIGITS "${Python_VERSION_MAJOR}" "${Python_VERSION_MINOR}")
endif()

if(_SCIRUN_PY_DIGITS)
  set(_PYTHON_NAME "python${_SCIRUN_PY_DIGITS}")
endif()

# Compute parent of site-packages using Python (optional)
set(_PYTHON_MODULE_SEARCH_PARENT "")
if(_PY_EXECUTABLE AND EXISTS "${_PY_EXECUTABLE}")
  set(_SCIRUN_PY_QUERY
    "import sysconfig; p=sysconfig.get_paths(); print((p.get('purelib') or p.get('platlib') or ''))"
  )
  execute_process(
    COMMAND "${_PY_EXECUTABLE}" -c "${_SCIRUN_PY_QUERY}"
    OUTPUT_VARIABLE _PY_SITE
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(_PY_SITE)
    get_filename_component(_PYTHON_MODULE_SEARCH_PARENT "${_PY_SITE}" DIRECTORY)
    file(TO_CMAKE_PATH "${_PYTHON_MODULE_SEARCH_PARENT}" _PYTHON_MODULE_SEARCH_PARENT)
  endif()
endif()

# Always define these macros; use empty string if unknown
if(WIN32)
  target_include_directories(Core_Python PRIVATE ${_PY_INCLUDE_DIRS})
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
    "PYTHONNAME=L\"${_PYTHON_NAME}\""
  )
elseif(APPLE)
  # Apple-specific legacy code paths could also need libdir; leave empty unless known
  target_include_directories(Core_Python PRIVATE ${_PY_INCLUDE_DIRS})
  target_compile_definitions(Core_Python PRIVATE
    "PYTHONPATH=L\"${_PYTHON_MODULE_SEARCH_PARENT}\""
    "PYTHONLIBDIR=L\"\""
    "PYTHONLIB=L\"\""
  )
else()
  target_include_directories(Core_Python PRIVATE ${_PY_INCLUDE_DIRS})
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
    ${SCI_BOOST_LIBRARY}              # keep existing boost meta-lib if your project uses it
    Core_Matlab
    Core_Datatypes
    Core_Datatypes_Legacy_Field
)

# --- Debug-only: ignore autolinked *release* python import lib (e.g., from Boost.Python) ---
# We already link the correct Debug import lib (pythonXY_d.lib) by full path.
# Compute the release filename to ignore (e.g., "python313.lib"), then pass /NODEFAULTLIB:<that>.
if(MSVC)
  # If digits were derived above (e.g., "313"), ignore exactly python313.lib.
  if(_SCIRUN_PY_DIGITS)
    set(_SCIRUN_PY_RELEASE_NAME "python${_SCIRUN_PY_DIGITS}.lib")
    target_link_options(Core_Python PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:${_SCIRUN_PY_RELEASE_NAME}>
    )
  else()
    # Fallback: also ignore the generic "python3.lib" name some toolchains request (rare).
    target_link_options(Core_Python PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:python3.lib>
    )
  endif()
endif()

# ====================================================================================
# Boost.Python handling
# - Recommended on MSVC: explicit linking to tolerate 'lib' prefix naming, vc toolset, and -gd debug suffix.
# - Otherwise: rely on MSVC autolink (ensure Boost lib dir is on link path).
# ====================================================================================

# Toggle (pass from superbuild via -DSCIRUN_EXPLICIT_BOOST_PYTHON_LINK=ON)
option(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK "Explicitly link Boost.Python on MSVC; otherwise rely on autolink" OFF)

# Digits for Boost.Python name (e.g., 313)
set(_SCIRUN_PY_M "${_SCIRUN_PY_DIGITS}")

if(MSVC)
  if(SCIRUN_EXPLICIT_BOOST_PYTHON_LINK)
    # Disable Boost autolink on this target
    target_compile_definitions(Core_Python PRIVATE BOOST_PYTHON_NO_LIB)
    #static
    target_compile_definitions(Core_Python PRIVATE BOOST_PYTHON_STATIC_LIB)

    # The Boost lib directory should be passed from the superbuild:
    #   -DSCI_BOOST_LIBRARY_DIR=E:/.../Externals/Install/Boost_external/lib
    if (NOT DEFINED SCI_BOOST_LIBRARY_DIR OR NOT EXISTS "${SCI_BOOST_LIBRARY_DIR}")
      message(FATAL_ERROR "SCI_BOOST_LIBRARY_DIR is not set or does not exist. Pass it from the superbuild.")
    endif()

    # Gather candidates for Debug (-gd-) with and without 'lib' prefix
    file(GLOB _bp_dbg_candidates
      "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib"
      "${SCI_BOOST_LIBRARY_DIR}/boost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib"
    )

    # Gather candidates for Release (no -gd-) with and without 'lib' prefix
    file(GLOB _bp_rel_candidates_all
      "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_M}-vc*-mt-*.lib"
      "${SCI_BOOST_LIBRARY_DIR}/boost_python${_SCIRUN_PY_M}-vc*-mt-*.lib"
    )
    # Filter out accidental debug files from the release list
    set(_bp_rel_candidates "")
    foreach(_cand IN LISTS _bp_rel_candidates_all)
      if(NOT _cand MATCHES "-gd-")
        list(APPEND _bp_rel_candidates "${_cand}")
      endif()
    endforeach()

    # Pick the most specific candidate (sort + take last)
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

    if(NOT SCI_BOOST_PYTHON_LIB_DEBUG OR NOT EXISTS "${SCI_BOOST_PYTHON_LIB_DEBUG}")
      message(FATAL_ERROR
        "Boost.Python Debug lib not found under '${SCI_BOOST_LIBRARY_DIR}'. "
        "Looked for libboost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib or boost_python${_SCIRUN_PY_M}-vc*-mt-gd-*.lib"
      )
    endif()
    if(NOT SCI_BOOST_PYTHON_LIB_RELEASE OR NOT EXISTS "${SCI_BOOST_PYTHON_LIB_RELEASE}")
      message(FATAL_ERROR
        "Boost.Python Release lib not found under '${SCI_BOOST_LIBRARY_DIR}'. "
        "Looked for libboost_python${_SCIRUN_PY_M}-vc*-mt-*.lib or boost_python${_SCIRUN_PY_M}-vc*-mt-*.lib"
      )
    endif()

    # Link the exact files (per-config)
    target_link_libraries(Core_Python PRIVATE
      debug     "${SCI_BOOST_PYTHON_LIB_DEBUG}"
      optimized "${SCI_BOOST_PYTHON_LIB_RELEASE}"
    )

    message(STATUS "[Core/Python] Explicitly linking Boost.Python:")
    message(STATUS "  Debug   : ${SCI_BOOST_PYTHON_LIB_DEBUG}")
    message(STATUS "  Release : ${SCI_BOOST_PYTHON_LIB_RELEASE}")
  else()
    # Keep autolink, but help the linker find the folder
    if (DEFINED SCI_BOOST_LIBRARY_DIR AND EXISTS "${SCI_BOOST_LIBRARY_DIR}")
      target_link_directories(Core_Python PRIVATE "${SCI_BOOST_LIBRARY_DIR}")
    endif()
    message(STATUS "[Core/Python] Using MSVC autolink for Boost.Python (ensure lib names match).")
    # For debugging the exact name autolink requests, uncomment:
    # target_compile_definitions(Core_Python PRIVATE BOOST_LIB_DIAGNOSTIC)
  endif()
else()
  # Non-Windows: explicitly link Boost.Python by name if available
  set(_bp_names
    "boost_python${_SCIRUN_PY_M}"
    "boost_python3"
    "boost_python"
  )
  set(_bp_lib "")

  # Prefer superbuild lib dir if present
  if(DEFINED SCI_BOOST_LIBRARY_DIR AND EXISTS "${SCI_BOOST_LIBRARY_DIR}")
    find_library(BOOST_PYTHON_LIB
      NAMES ${_bp_names}
      PATHS "${SCI_BOOST_LIBRARY_DIR}" "${SCI_BOOST_PREFIX}/lib" "${SCI_BOOST_PREFIX}/lib64"
      NO_DEFAULT_PATH
    )
    set(_bp_lib "${BOOST_PYTHON_LIB}")
  endif()

  # Fallback to default search paths
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

# Debug-only macro (matches your previous behavior)
set_target_properties(Core_Python
  PROPERTIES
    COMPILE_DEFINITIONS_DEBUG "BOOST_DEBUG_PYTHON"
)

# Helpful diagnostics
message(STATUS "[Core/Python] PY includes : ${_PY_INCLUDE_DIRS}")
message(STATUS "[Core/Python] PY exec     : ${_PY_EXECUTABLE}")
message(STATUS "[Core/Python] PY name     : ${_PYTHON_NAME}")
message(STATUS "[Core/Python] PY path     : ${_PYTHON_MODULE_SEARCH_PARENT}")
if(_PY_LIB_DBG)
  message(STATUS "[Core/Python] PY lib (dbg): ${_PY_LIB_DBG}")
endif()
if(_PY_LIB_REL)
  message(STATUS "[Core/Python] PY lib (rel): ${_PY_LIB_REL}")
endif()

# --- DIAGNOSTICS: show who is adding system Python into Core_Python ---
get_target_property(_cp_links Core_Python LINK_LIBRARIES)
get_target_property(_cp_if    Core_Python INTERFACE_LINK_LIBRARIES)
message(STATUS "[Diag/Core_Python] LINK_LIBRARIES=${_cp_links}")
message(STATUS "[Diag/Core_Python] INTERFACE_LINK_LIBRARIES=${_cp_if}")
message(STATUS "[Diag/Core_Python] Python::Python IMPORTED_IMPLIB_DEBUG=$<TARGET_PROPERTY:Python::Python,IMPORTED_IMPLIB_DEBUG>")
message(STATUS "[Diag/Core_Python] Python::Python IMPORTED_IMPLIB_RELEASE=$<TARGET_PROPERTY:Python::Python,IMPORTED_IMPLIB_RELEASE>")