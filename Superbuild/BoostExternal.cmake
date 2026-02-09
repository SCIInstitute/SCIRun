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
)

# Optional compiled libs (usually OFF)
option(BOOST_ENABLE_PYTHON "Build Boost.Python if Python is available" OFF)
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

# ========= Build knobs for b2 (must match your app's toolchain) =========
# STATIC vs SHARED Boost libs (import lib + DLL if SHARED)
option(BOOST_USE_STATIC_LIBS "Build static Boost libraries" ON)

# Runtime library (/MD,/MDd shared; /MT,/MTd static)
option(BOOST_USE_STATIC_RUNTIME "Use static C runtime (/MT,/MTd). OFF = /MD,/MDd" OFF)

# MSVC toolset (VS 2022 ~ 14.3). Override with -DB2_TOOLSET=msvc-14.3 if needed.
if(NOT DEFINED B2_TOOLSET)
  set(B2_TOOLSET msvc-14.3)
endif()

# ========= ExternalProject definition =========
ExternalProject_Add(Boost_external
  GIT_REPOSITORY            ${_boost_git_url}
  GIT_TAG                   ${_boost_git_tag}
  GIT_SHALLOW               FALSE
  GIT_PROGRESS              TRUE

  # Ensure submodules are available
  UPDATE_COMMAND            ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git submodule update --init --recursive

  BUILD_IN_SOURCE           OFF

  # We will NOT use the CMake superproject for compiled libs
  CONFIGURE_COMMAND         ""
  BUILD_COMMAND             ""
  INSTALL_COMMAND           ""
)

ExternalProject_Get_Property(Boost_external SOURCE_DIR)
ExternalProject_Get_Property(Boost_external BINARY_DIR)
ExternalProject_Get_Property(Boost_external INSTALL_DIR)

# ========= Cross-platform b2 header staging =========
# 1) Bootstrap b2
if(WIN32)
  set(_B2_BOOTSTRAP_CMD cmd /c bootstrap.bat)
  set(_B2_CMD           cmd /c .\\b2)
else()
  set(_B2_BOOTSTRAP_CMD ./bootstrap.sh)
  set(_B2_CMD           ./b2)
endif()

ExternalProject_Add_Step(Boost_external bootstrap_b2
  COMMAND ${_B2_BOOTSTRAP_CMD}
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES update
  COMMENT "Bootstrapping Boost.Build (b2)"
)

# 2) Generate the full 'boost/' header tree
ExternalProject_Add_Step(Boost_external stage_headers
  COMMAND ${_B2_CMD} headers
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES bootstrap_b2
  COMMENT "Running 'b2 headers' to generate the boost/ header tree"
)

# ========= Build compiled libs with b2 (MSVC-style versioned names) =========
# Determine b2 arguments
if(BOOST_USE_STATIC_LIBS)
  set(_B2_LINK "link=static")
else()
  set(_B2_LINK "link=shared")
endif()

if(BOOST_USE_STATIC_RUNTIME)
  set(_B2_RUNTIME_LINK "runtime-link=static")   # /MT,/MTd
else()
  set(_B2_RUNTIME_LINK "runtime-link=shared")   # /MD,/MDd
endif()

set(_B2_VARIANTS "variant=debug,release")       # build both for multi-config IDEs

# Keep all b2 artifacts under <BINARY_DIR>/b2-build
set(_B2_BUILD_DIR ${BINARY_DIR}/b2-build)

ExternalProject_Add_Step(Boost_external build_b2_libs
  COMMAND ${_B2_CMD}
          -j${CMAKE_BUILD_PARALLEL_LEVEL}
          address-model=64 architecture=x86
          toolset=${B2_TOOLSET}
          threading=multi
          ${_B2_LINK}
          ${_B2_RUNTIME_LINK}
          ${_B2_VARIANTS}
          --layout=versioned
          ${_BOOST_LIBS_B2}
          --build-dir=${_B2_BUILD_DIR}
          stage
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES stage_headers
  COMMENT "Building Boost libs with b2 (versioned names for MSVC auto-link)"
)

# 3) Install staged libs into <INSTALL_DIR>/lib
ExternalProject_Add_Step(Boost_external install_b2_libs
  COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/lib
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/stage/lib <INSTALL_DIR>/lib
  DEPENDEES build_b2_libs
  COMMENT "Installing b2-built Boost libs into <INSTALL_DIR>/lib"
)

# 4) Copy the entire header tree into the install prefix
ExternalProject_Add_Step(Boost_external install_full_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
  COMMAND ${CMAKE_COMMAND} -E remove_directory <INSTALL_DIR>/include/boost
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/boost <INSTALL_DIR>/include/boost
  DEPENDEES stage_headers
  COMMENT "Copying full Boost headers to <INSTALL_DIR>/include/boost"
)

# ========= Export properties for downstream =========
# Convenience prefix
set(SCI_BOOST_PREFIX "${INSTALL_DIR}")

# Concrete include/lib paths (SCIRun consumes these)
set(SCI_BOOST_INCLUDE "${SCI_BOOST_PREFIX}/include" CACHE PATH "Boost include directory" FORCE)
if (EXISTS "${SCI_BOOST_PREFIX}/lib64")
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib64")
else()
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib")
endif()
set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_LIBRARY_DIR}" CACHE PATH "Boost library directory" FORCE)

# Since we are NOT installing BoostConfig.cmake, prefer MODULE mode if you still call find_package(Boost)
set(Boost_DIR "" CACHE PATH "No BoostConfig.cmake (b2 build). Use FindBoost (MODULE) if needed." FORCE)
set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Force FindBoost MODULE mode" FORCE)
set(Boost_ROOT "${SCI_BOOST_PREFIX}" CACHE PATH "Boost install prefix" FORCE)
set(BOOST_ROOT "${SCI_BOOST_PREFIX}" CACHE PATH "Boost install prefix (compat)" FORCE)
set(BOOST_LIBRARYDIR "${SCI_BOOST_LIBRARY_DIR}" CACHE PATH "Boost library dir (compat)" FORCE)

# ========= Generate a 'UseBoost.cmake' for consumers (auto-link ON) =========
# - Adds include and link search dirs.
# - If building shared Boost libs, define BOOST_ALL_DYN_LINK (ensures __declspec(dllimport)).
# - INTENTIONALLY does NOT define BOOST_ALL_NO_LIB (we want auto-link to inject versioned names).
set(SCI_BOOST_USE_FILE "${SCI_BOOST_PREFIX}/UseBoost.cmake")
set(_usefile "## Auto-generated UseBoost.cmake (b2 + autolink)
# Include dirs for headers
include_guard(GLOBAL)
if(EXISTS \"${SCI_BOOST_INCLUDE}\")
  include_directories(\"${SCI_BOOST_INCLUDE}\")
endif()

# Link search dir so MSVC can find the versioned .lib names injected by auto-link
if(EXISTS \"${SCI_BOOST_LIBRARY_DIR}\")
  link_directories(\"${SCI_BOOST_LIBRARY_DIR}\")
endif()

# If Boost was built as shared libs, define BOOST_ALL_DYN_LINK so import libs/DLLs are used
")
if(NOT BOOST_USE_STATIC_LIBS)
  string(APPEND _usefile "add_compile_definitions(BOOST_ALL_DYN_LINK)\n")
endif()

# Helpful hints for FindBoost (MODULE) if you still want to query variables
string(APPEND _usefile "
set(Boost_ROOT \"${SCI_BOOST_PREFIX}\")
set(BOOST_ROOT  \"${SCI_BOOST_PREFIX}\")
set(BOOST_LIBRARYDIR \"${SCI_BOOST_LIBRARY_DIR}\")
")
file(WRITE "${SCI_BOOST_USE_FILE}" "${_usefile}")

# ========= Diagnostics =========
message(STATUS "[Boost_ext] INSTALL_DIR: ${SCI_BOOST_PREFIX}")
message(STATUS "[Boost_ext] Include dir: ${SCI_BOOST_INCLUDE}")
message(STATUS "[Boost_ext] Lib dir:     ${SCI_BOOST_LIBRARY_DIR}")
message(STATUS "[Boost_ext] Built libs:  ${_BOOST_LIBS}")
message(STATUS "[Boost_ext] b2 link type:         ${_B2_LINK}")
message(STATUS "[Boost_ext] b2 runtime link:      ${_B2_RUNTIME_LINK}")
message(STATUS "[Boost_ext] b2 toolset:           ${B2_TOOLSET}")
message(STATUS "[Boost_ext] Use file:             ${SCI_BOOST_USE_FILE}")
message(STATUS "[Boost_ext] Note: No BoostConfig.cmake installed (using b2). Auto-link on MSVC will inject versioned .lib names.")