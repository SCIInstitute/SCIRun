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

# Optional: make CMake report condition errors clearly (silences the dev warning)
if (POLICY CMP0130)
  cmake_policy(SET CMP0130 NEW)
endif()

# ---- Sources ----
set(Core_ConsoleApplication_SRCS
  ConsoleApplication.cc
  ConsoleCommandFactory.cc
  ConsoleCommands.cc
)

set(Core_ConsoleApplication_HEADERS
  ConsoleApplication.h
  ConsoleCommandFactory.h
  ConsoleCommands.h
  share.h
)

# ---- Target ----
SCIRUN_ADD_LIBRARY(Core_ConsoleApplication
  ${Core_ConsoleApplication_HEADERS}
  ${Core_ConsoleApplication_SRCS}
)

# Disable Boost header auto-linking on this target (prevents implicit /DEFAULTLIB from headers)
target_compile_definitions(Core_ConsoleApplication PRIVATE
  BOOST_ALL_NO_LIB BOOST_PYTHON_NO_LIB
)

# ---- Build a local Boost list WITHOUT Boost.Python, preserving debug/optimized pairs ----
set(_src_boost_list "${SCI_BOOST_LIBRARY}")
set(_boost_libs_filtered "")

# Match any Boost.Python library variant
set(_boost_python_rx ".*[Bb]oost[_-]python.*\\.lib$")

list(LENGTH _src_boost_list _n)
set(_i 0)
while(_i LESS _n)
  list(GET _src_boost_list ${_i} _item)

  if((_item STREQUAL "debug") OR (_item STREQUAL "optimized"))
    # Ensure there is a following library for the keyword
    math(EXPR _j "${_i} + 1")
    if(_j LESS _n)
      list(GET _src_boost_list ${_j} _lib)
      if(_lib MATCHES "${_boost_python_rx}")
        # Skip the pair (keyword + python lib)
        math(EXPR _i "${_i} + 2")
        continue()
      else()
        # Keep both keyword and library
        list(APPEND _boost_libs_filtered "${_item}" "${_lib}")
        math(EXPR _i "${_i} + 2")
        continue()
      endif()
    else()
      # Dangling keyword (defensive)
      math(EXPR _i "${_i} + 1")
      continue()
    endif()
  else()
    # Non-keyword item
    if(_item MATCHES "${_boost_python_rx}")
      # Skip unqualified python lib
      math(EXPR _i "${_i} + 1")
      continue()
    else()
      list(APPEND _boost_libs_filtered "${_item}")
      math(EXPR _i "${_i} + 1")
      continue()
    endif()
  endif()
endwhile()

# Optional diagnostics
message(STATUS "[Core/ConsoleApplication] Boost libs after filtering: ${_boost_libs_filtered}")

# ---- Link base deps + filtered Boost (no Boost.Python) ----
# Use keyword form consistently to avoid the mixed-signature error
target_link_libraries(Core_ConsoleApplication PRIVATE
  Core_Application
  Core_Command
  ${_boost_libs_filtered}
  LodePNG::lodepng
)

# ---- Python includes (unchanged from your version) ----
if (BUILD_WITH_PYTHON)
  set(_PY_INCLUDES "")
  if (DEFINED PYTHON_INCLUDE_DIR AND EXISTS "${PYTHON_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_INCLUDE_DIR}")
  endif()
  if (DEFINED PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_PC_INCLUDE_DIR}")
  endif()
  if (MSVC AND NOT _PY_INCLUDES)
    set(_py_src "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external")
    if (EXISTS "${_py_src}/Include")
      list(APPEND _PY_INCLUDES "${_py_src}/Include")
    endif()
    if (EXISTS "${_py_src}/PC")
      list(APPEND _PY_INCLUDES "${_py_src}/PC")
    endif()
  endif()
  if (_PY_INCLUDES)
    list(REMOVE_DUPLICATES _PY_INCLUDES)
    target_include_directories(Core_ConsoleApplication PRIVATE ${_PY_INCLUDES})
    message(STATUS "[Core/ConsoleApplication] Added Python includes: ${_PY_INCLUDES}")
  else()
    message(WARNING "[Core/ConsoleApplication] Python includes not found at configure time; Boost.Python compilation may fail.")
  endif()
endif()

# ---- Link Core_Python if requested (your curated aggregation) ----
if (BUILD_WITH_PYTHON)
  target_link_libraries(Core_ConsoleApplication PRIVATE Core_Python)
endif()

# ---- Force the correct Python import lib per configuration & ignore wrong defaultlibs ----
if (MSVC AND BUILD_WITH_PYTHON)
  # Neutralize wrong defaultlibs embedded by dependent .libs (e.g., Boost.Python)
  target_link_options(Core_ConsoleApplication PRIVATE
    $<$<CONFIG:Debug>:/NODEFAULTLIB:python313.lib>
    $<$<CONFIG:Release>:/NODEFAULTLIB:python313_d.lib>
    $<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:python313_d.lib>
    $<$<CONFIG:MinSizeRel>:/NODEFAULTLIB:python313_d.lib>
  )

  # Prefer your imported Python target (from superbuild helper)
  if (TARGET SCIRunPython::Python)
    target_link_libraries(Core_ConsoleApplication PRIVATE
      $<$<CONFIG:Debug>:SCIRunPython::Python>
      $<$<NOT:$<CONFIG:Debug>>:SCIRunPython::Python>
    )
  elseif (TARGET Python::Python)
    target_link_libraries(Core_ConsoleApplication PRIVATE
      $<$<CONFIG:Debug>:Python::Python>
      $<$<NOT:$<CONFIG:Debug>>:Python::Python>
    )
  else()
    # Fallback to explicit path probing for python313[_d].lib within superbuild
    set(_PY_DBG "")
    set(_PY_REL "")
    foreach(dir
      "${CMAKE_BINARY_DIR}/../Externals/Install/Python_external/libs"
      "${CMAKE_BINARY_DIR}/../Externals/Install/Python_external/lib"
      "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external/PCbuild/amd64"
    )
      if (EXISTS "${dir}/python313_d.lib" AND NOT _PY_DBG)
        set(_PY_DBG "${dir}/python313_d.lib")
      endif()
      if (EXISTS "${dir}/python313.lib" AND NOT _PY_REL)
        set(_PY_REL "${dir}/python313.lib")
      endif()
    endforeach()

    if (_PY_DBG)
      target_link_libraries(Core_ConsoleApplication PRIVATE
        $<$<CONFIG:Debug>:${_PY_DBG}>
      )
      message(STATUS "[Core/ConsoleApplication] Using ${_PY_DBG} for Debug")
    else()
      message(WARNING "[Core/ConsoleApplication] Could not locate python313_d.lib; Debug link may still fail.")
    endif()

    if (_PY_REL)
      target_link_libraries(Core_ConsoleApplication PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:${_PY_REL}>
      )
      message(STATUS "[Core/ConsoleApplication] Using ${_PY_REL} for non-Debug")
    else()
      message(WARNING "[Core/ConsoleApplication] Could not locate python313.lib; non-Debug link may fail.")
    endif()
  endif()
endif()

# ---- Shared libs define ----
if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Core_ConsoleApplication)
endif()