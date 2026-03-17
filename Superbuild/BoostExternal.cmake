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
# Build ONLY the compiled libraries SCIRun needs to link.
# We will generate + install the FULL header tree separately via `b2 headers`.
set(_BOOST_LIBS
  filesystem
  system
  atomic
  thread
  program_options
  serialization
)

# Optional compiled libs (usually OFF)
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

# Convert to b2 --with-<lib> arguments
set(_BOOST_LIBS_B2 "")
foreach(_bl IN LISTS _BOOST_LIBS)
  list(APPEND _BOOST_LIBS_B2 "--with-${_bl}")
endforeach()

# ========= Cross‑platform Boost toolset detection =========
# On Windows   → use msvc
# On macOS     → use clang-darwin (Boost’s preferred Darwin toolset)
# On Linux     → use gcc or clang depending on CMAKE_CXX_COMPILER_ID

if(WIN32)
  set(_B2_TOOLSET "msvc")
elseif(APPLE)
  # Boost official recommended toolset on macOS
  set(_B2_TOOLSET "clang-darwin")
else() # Linux / Unix
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(_B2_TOOLSET "gcc")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(_B2_TOOLSET "clang")
  else()
    message(FATAL_ERROR "Unsupported compiler '${CMAKE_CXX_COMPILER_ID}' for Boost build")
  endif()
endif()

message(STATUS "[Boost_ext] Auto-selected b2 toolset: ${_B2_TOOLSET}")

# Allow manual override
if(DEFINED B2_TOOLSET)
  message(STATUS "[Boost_ext] Overriding toolset with user B2_TOOLSET=${B2_TOOLSET}")
  set(_B2_TOOLSET "${B2_TOOLSET}")
endif()

# ========= Linkage settings =========
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

# ========= b2 executable paths =========
if(WIN32)
  set(_B2_BOOTSTRAP_CMD cmd /c bootstrap.bat)
  set(_B2_CMD           cmd /c .\\b2)
else()
  set(_B2_BOOTSTRAP_CMD ./bootstrap.sh)
  set(_B2_CMD           ./b2)
endif()

# ========= Build directory for b2 artifacts =========
set(_B2_BUILD_DIR ${BINARY_DIR}/b2-build)

# ========= Build command (cross‑platform) =========
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
  COMMENT "Building Boost with b2 (cross‑platform toolset=${_B2_TOOLSET})"
)

# ========= Install libs =========
ExternalProject_Add_Step(Boost_external install_b2_libs
  COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/lib
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/stage/lib <INSTALL_DIR>/lib
  DEPENDEES build_b2_libs
  COMMENT "Installing Boost libs"
)

# ========= Install headers =========
ExternalProject_Add_Step(Boost_external install_full_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
  COMMAND ${CMAKE_COMMAND} -E remove_directory <INSTALL_DIR>/include/boost
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/boost <INSTALL_DIR>/include/boost
  DEPENDEES stage_headers
  COMMENT "Installing Boost headers"
)