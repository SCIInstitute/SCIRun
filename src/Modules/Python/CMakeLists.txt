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
# Python wiring:
#   - Reuse alias imported target SCIRunPython (created in Core/Python) that points
#     to .../PCbuild/amd64/python313[_d].lib and exports Python include dirs.
#   - If it does not exist (edge case), create a compatible local alias.
# ====================================================================================
if(BUILD_WITH_PYTHON)
  if(NOT TARGET SCIRunPython)
    # Minimal local alias (fallback) — prefer superbuild hints if available
    add_library(SCIRunPython STATIC IMPORTED GLOBAL)

    # Includes: prefer superbuild vars
    set(_PY_INC_DIRS "")
    if(DEFINED PYTHON_INCLUDE_DIR AND EXISTS "${PYTHON_INCLUDE_DIR}")
      list(APPEND _PY_INC_DIRS "${PYTHON_INCLUDE_DIR}")
    endif()
    if(DEFINED PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
      list(APPEND _PY_INC_DIRS "${PYTHON_PC_INCLUDE_DIR}")
    endif()
    if(MSVC AND NOT _PY_INC_DIRS)
      set(_py_src "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external")
      if(EXISTS "${_py_src}/Include")
        list(APPEND _PY_INC_DIRS "${_py_src}/Include")
      endif()
      if(EXISTS "${_py_src}/PC")
        list(APPEND _PY_INC_DIRS "${_py_src}/PC")
      endif()
    endif()
    if(_PY_INC_DIRS)
      list(REMOVE_DUPLICATES _PY_INC_DIRS)
      set_target_properties(SCIRunPython PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_PY_INC_DIRS}"
      )
    endif()

    # Point to import libs on MSVC
    if(MSVC)
      set_target_properties(SCIRunPython PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
      )
      set(_PY_LIB_REL "")
      set(_PY_LIB_DBG "")
      if(PYTHON_LIBRARY_RELEASE AND EXISTS "${PYTHON_LIBRARY_RELEASE}")
        set(_PY_LIB_REL "${PYTHON_LIBRARY_RELEASE}")
      elseif(DEFINED PY_EXT_LIB_DIR AND EXISTS "${PY_EXT_LIB_DIR}/python313.lib")
        set(_PY_LIB_REL "${PY_EXT_LIB_DIR}/python313.lib")
      endif()
      if(PYTHON_LIBRARY_DEBUG AND EXISTS "${PYTHON_LIBRARY_DEBUG}")
        set(_PY_LIB_DBG "${PYTHON_LIBRARY_DEBUG}")
      elseif(DEFINED PY_EXT_LIB_DIR AND EXISTS "${PY_EXT_LIB_DIR}/python313_d.lib")
        set(_PY_LIB_DBG "${PY_EXT_LIB_DIR}/python313_d.lib")
      endif()

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

      # Map if only one variant exists
      if(NOT _PY_LIB_DBG AND _PY_LIB_REL)
        set_target_properties(SCIRunPython PROPERTIES
          IMPORTED_LOCATION_DEBUG "${_PY_LIB_REL}"
          MAP_IMPORTED_CONFIG_DEBUG "Release;RelWithDebInfo"
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
    endif()
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
  # Pull in Python headers + import libs (per-config) via the alias
  target_link_libraries(Modules_Python PRIVATE SCIRunPython Core_Python)

  # ==================================================================================
  # Boost.Python: disable autolink and link the correct libraries explicitly on MSVC.
  # Fixes LNK1104 asking for 'boost_python...' (no 'lib' prefix) when your files are
  # named 'libboost_python...'.
  # ==================================================================================
  if(MSVC)
    target_compile_definitions(Modules_Python PRIVATE BOOST_PYTHON_NO_LIB BOOST_PYTHON_STATIC_LIB)

    # Locate Boost.Python libraries in SCI_BOOST_LIBRARY_DIR (passed from superbuild)
    if (NOT DEFINED SCI_BOOST_LIBRARY_DIR OR NOT EXISTS "${SCI_BOOST_LIBRARY_DIR}")
      message(FATAL_ERROR
        "[Modules/Python] SCI_BOOST_LIBRARY_DIR is not set or does not exist; "
        "cannot link Boost.Python explicitly."
      )
    endif()

    # Try to infer python digits (e.g., 313) from the CPython import libs we are using
    set(_SCIRUN_PY_DIGITS "")
    if(TARGET SCIRunPython)
      get_target_property(_py_dbg SCIRunPython IMPORTED_LOCATION_DEBUG)
      get_target_property(_py_rel SCIRunPython IMPORTED_LOCATION_RELEASE)
      foreach(_cand "${_py_dbg}" "${_py_rel}")
        if(_cand)
          get_filename_component(_bn "${_cand}" NAME)
          if(_bn MATCHES "python([0-9]+)(_d)?\\.lib$")
            set(_SCIRUN_PY_DIGITS "${CMAKE_MATCH_1}")
            break()
          endif()
        endif()
      endforeach()
    endif()

    # Build candidate patterns (both with/without 'lib' prefix)
    if(_SCIRUN_PY_DIGITS)
      set(_bp_dbg_patterns
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_DIGITS}-vc*-mt-gd-*.lib"
        "${SCI_BOOST_LIBRARY_DIR}/boost_python${_SCIRUN_PY_DIGITS}-vc*-mt-gd-*.lib"
      )
      set(_bp_rel_patterns_all
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python${_SCIRUN_PY_DIGITS}-vc*-mt-*.lib"
        "${SCI_BOOST_LIBRARY_DIR}/boost_python${_SCIRUN_PY_DIGITS}-vc*-mt-*.lib"
      )
    else()
      # Fallback: any 3.x debug/release variants
      set(_bp_dbg_patterns
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python3*-vc*-mt-gd-*.lib"
        "${SCI_BOOST_LIBRARY_DIR}/boost_python3*-vc*-mt-gd-*.lib"
      )
      set(_bp_rel_patterns_all
        "${SCI_BOOST_LIBRARY_DIR}/libboost_python3*-vc*-mt-*.lib"
        "${SCI_BOOST_LIBRARY_DIR}/boost_python3*-vc*-mt-*.lib"
      )
    endif()

    # Collect and pick best (latest) debug candidate
    set(_bp_dbg_candidates "")
    foreach(_pat IN LISTS _bp_dbg_patterns)
      file(GLOB _hits "${_pat}")
      if(_hits)
        list(APPEND _bp_dbg_candidates ${_hits})
      endif()
    endforeach()
    if(_bp_dbg_candidates)
      list(SORT _bp_dbg_candidates)
      list(REVERSE _bp_dbg_candidates)
      list(GET _bp_dbg_candidates 0 _BP_PY_LIB_DEBUG)
    endif()

    # Collect and pick best (latest) release candidate, but filter out "-gd-"
    set(_bp_rel_candidates "")
    foreach(_pat IN LISTS _bp_rel_patterns_all)
      file(GLOB _hits "${_pat}")
      foreach(_h IN LISTS _hits)
        if(NOT _h MATCHES "-gd-")  # exclude debug variants
          list(APPEND _bp_rel_candidates "${_h}")
        endif()
      endforeach()
    endforeach()
    if(_bp_rel_candidates)
      list(SORT _bp_rel_candidates)
      list(REVERSE _bp_rel_candidates)
      list(GET _bp_rel_candidates 0 _BP_PY_LIB_RELEASE)
    endif()

    if(NOT (EXISTS "${_BP_PY_LIB_DEBUG}" AND EXISTS "${_BP_PY_LIB_RELEASE}"))
      message(FATAL_ERROR
        "[Modules/Python] Could not locate Boost.Python libs under '${SCI_BOOST_LIBRARY_DIR}'.\n"
        "Debug candidate='${_BP_PY_LIB_DEBUG}'\nRelease candidate='${_BP_PY_LIB_RELEASE}'"
      )
    endif()

    # Ensure the directory is on the link path
    target_link_directories(Modules_Python PRIVATE "${SCI_BOOST_LIBRARY_DIR}")

    # Explicitly link Boost.Python (eliminates autolink name mismatch)
    target_link_libraries(Modules_Python PRIVATE
      debug     "${_BP_PY_LIB_DEBUG}"
      optimized "${_BP_PY_LIB_RELEASE}"
    )

    # Debug-only: ignore any attempt to autolink the *release* python313.lib
    if(_SCIRUN_PY_DIGITS)
      target_link_options(Modules_Python PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:python${_SCIRUN_PY_DIGITS}.lib>
      )
    else()
      target_link_options(Modules_Python PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:python3.lib>
      )
    endif()

    # Diagnostics
    message(STATUS "[Modules/Python] Boost.Python (debug)   : ${_BP_PY_LIB_DEBUG}")
    message(STATUS "[Modules/Python] Boost.Python (release) : ${_BP_PY_LIB_RELEASE}")
  endif() # MSVC

  target_compile_definitions(Modules_Python PRIVATE BUILD_WITH_PYTHON)
endif() # BUILD_WITH_PYTHON

# ====================================================================================
# Tests
# ====================================================================================
if(BUILD_WITH_PYTHON)
  SCIRUN_ADD_TEST_DIR(Tests)
endif()