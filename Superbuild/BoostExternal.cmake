#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
#  University of Utah.
#
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

# --- Inputs ---
set(_boost_git_url "https://github.com/CIBC-Internal/boost.git")
set(_boost_git_tag "v1.90.0")

include(ExternalProject)

# ========= Compiled library selection =========
set(_BOOST_LIBS
  filesystem
  system
  atomic
  thread
  program_options
  serialization
)

option(BOOST_ENABLE_PYTHON "Build Boost.Python if Python is available" ON)
option(BOOST_ENABLE_MPI    "Build Boost.MPI if MPI is available"       OFF)
option(BOOST_ENABLE_TEST   "Build Boost.Test (unit test framework)"    OFF)

if(BOOST_ENABLE_PYTHON)
  list(APPEND _BOOST_LIBS python)
endif()
if(BOOST_ENABLE_MPI)
  list(APPEND _BOOST_LIBS mpi graph_parallel)
endif()
if(BOOST_ENABLE_TEST)
  list(APPEND _BOOST_LIBS test)
endif()

set(_BOOST_LIBS_B2 "")
foreach(_bl IN LISTS _BOOST_LIBS)
  list(APPEND _BOOST_LIBS_B2 "--with-${_bl}")
endforeach()

# ============================================================
# 1) Define the external project FIRST (required by CMake)
# ============================================================
ExternalProject_Add(Boost_external
  GIT_REPOSITORY  ${_boost_git_url}
  GIT_TAG         ${_boost_git_tag}
  GIT_SHALLOW     FALSE
  GIT_PROGRESS    TRUE

  UPDATE_COMMAND  ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git submodule update --init --recursive

  BUILD_IN_SOURCE OFF

  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
)

# ============================================================
# 2) Retrieve SOURCE_DIR, BINARY_DIR, INSTALL_DIR
# ============================================================
ExternalProject_Get_Property(Boost_external SOURCE_DIR)
ExternalProject_Get_Property(Boost_external BINARY_DIR)
ExternalProject_Get_Property(Boost_external INSTALL_DIR)

# ============================================================
# 3) Optional Boost.Python config
# ============================================================
set(_B2_PY_ARGS "")
if(BOOST_ENABLE_PYTHON)
  set(_PY_LIBDIR "")

  if(DEFINED PYTHON_LIBRARY_DEBUG AND EXISTS "${PYTHON_LIBRARY_DEBUG}")
    get_filename_component(_PY_LIBDIR "${PYTHON_LIBRARY_DEBUG}" DIRECTORY)
  elseif(DEFINED PYTHON_LIBRARY_RELEASE AND EXISTS "${PYTHON_LIBRARY_RELEASE}")
    get_filename_component(_PY_LIBDIR "${PYTHON_LIBRARY_RELEASE}" DIRECTORY)
  elseif(DEFINED PYTHON_RUNTIME_DIR AND EXISTS "${PYTHON_RUNTIME_DIR}")
    set(_PY_LIBDIR "${PYTHON_RUNTIME_DIR}")
  endif()

  if(PYTHON_INCLUDE_DIR AND EXISTS "${PYTHON_INCLUDE_DIR}" AND
     _PY_LIBDIR AND EXISTS "${_PY_LIBDIR}")
    list(APPEND _B2_PY_ARGS
      "include=${PYTHON_INCLUDE_DIR}"
      "library-path=${_PY_LIBDIR}"
    )
    message(STATUS "[Boost_ext] Will build Boost.Python against include='${PYTHON_INCLUDE_DIR}', libdir='${_PY_LIBDIR}'")
  else()
    message(WARNING "[Boost_ext] BOOST_ENABLE_PYTHON=ON but Python paths incomplete.")
  endif()
endif()

# ============================================================
# 4) Cross‑platform Boost toolset detection
# ============================================================
if(WIN32)
  set(_B2_TOOLSET "msvc")
elseif(APPLE)
  set(_B2_TOOLSET "clang-darwin")
else()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(_B2_TOOLSET "gcc")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(_B2_TOOLSET "clang")
  else()
    message(FATAL_ERROR "Unsupported compiler '${CMAKE_CXX_COMPILER_ID}' for Boost build")
  endif()
endif()

message(STATUS "[Boost_ext] Auto-selected b2 toolset: ${_B2_TOOLSET}")

if(DEFINED B2_TOOLSET)
  message(STATUS "[Boost_ext] Overriding toolset with B2_TOOLSET=${B2_TOOLSET}")
  set(_B2_TOOLSET "${B2_TOOLSET}")
endif()

# ============================================================
# 5) Link options
# ============================================================
if(BOOST_USE_STATIC_LIBS)
  set(_B2_LINK "link=static")
else()
  set(_B2_LINK "link=shared")
endif()

if(BOOST_USE_STATIC_RUNTIME)
  set(_B2_RUNTIME_LINK "runtime-link=static")
else()
  set(_B2_RUNTIME_LINK "runtime-link=shared")
endif()

set(_B2_VARIANTS "variant=debug,release")

# ============================================================
# 6) b2 executable and build dir
# ============================================================
if(WIN32)
  set(_B2_BOOTSTRAP_CMD cmd /c bootstrap.bat)
  set(_B2_CMD           cmd /c .\\b2)
else()
  set(_B2_BOOTSTRAP_CMD ./bootstrap.sh)
  set(_B2_CMD           ./b2)
endif()

set(_B2_BUILD_DIR ${BINARY_DIR}/b2-build)

# ============================================================
# 7) bootstrap b2
# ============================================================
ExternalProject_Add_Step(Boost_external bootstrap_b2
  COMMAND ${_B2_BOOTSTRAP_CMD}
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES update
  COMMENT "Bootstrapping b2"
)

# ============================================================
# 8) Run b2 headers
# ============================================================
ExternalProject_Add_Step(Boost_external stage_headers
  COMMAND ${_B2_CMD} headers
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES bootstrap_b2
  COMMENT "Generating Boost headers (b2 headers)"
)

# ============================================================
# 9) Build Boost libraries
# ============================================================
ExternalProject_Add_Step(Boost_external build_b2_libs
  COMMAND ${_B2_CMD}
          -j${CMAKE_BUILD_PARALLEL_LEVEL}
          toolset=${_B2_TOOLSET}
          threading=multi
          ${_B2_LINK}
          ${_B2_RUNTIME_LINK}
          ${_B2_VARIANTS}
          --layout=tagged
          ${_BOOST_LIBS_B2}
          ${_B2_PY_ARGS}
          --build-dir=${_B2_BUILD_DIR}
          stage
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES stage_headers
  COMMENT "Building Boost libs"
)

# ============================================================
# 10) Install b2-built libs
# ============================================================
ExternalProject_Add_Step(Boost_external install_b2_libs
  COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/lib
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/stage/lib <INSTALL_DIR>/lib
  DEPENDEES build_b2_libs
  COMMENT "Installing Boost libs"
)

# ============================================================
# 11) Install full header tree
# ============================================================
ExternalProject_Add_Step(Boost_external install_full_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
  COMMAND ${CMAKE_COMMAND} -E remove_directory <INSTALL_DIR>/include/boost
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/boost <INSTALL_DIR>/include/boost
  DEPENDEES stage_headers
  COMMENT "Installing Boost headers"
)

# ============================================================
# 12) Export paths to superbuild
# ============================================================
set(SCI_BOOST_PREFIX "${INSTALL_DIR}")

set(SCI_BOOST_INCLUDE "${SCI_BOOST_PREFIX}/include" CACHE PATH "Boost include dir" FORCE)

if(EXISTS "${SCI_BOOST_PREFIX}/lib64")
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib64")
else()
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib")
endif()

set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_LIBRARY_DIR}" CACHE PATH "Boost library dir" FORCE)

set(Boost_DIR "" CACHE PATH "" FORCE)
set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "" FORCE)
set(Boost_ROOT "${SCI_BOOST_PREFIX}" CACHE PATH "" FORCE)
set(BOOST_ROOT "${SCI_BOOST_PREFIX}" CACHE PATH "" FORCE)
set(BOOST_LIBRARYDIR "${SCI_BOOST_LIBRARY_DIR}" CACHE PATH "" FORCE)