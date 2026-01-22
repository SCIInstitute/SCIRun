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

# ========= Library selection strategy =========
# Boost CMake superproject builds compiled libs only when listed in BOOST_INCLUDE_LIBRARIES.
# Header-only libs (e.g., signals2, asio, any, optional, variant, etc.) need no listing.

# A maximal "safe" set that builds everywhere without extra deps:
set(_BOOST_LIBS_SAFE
  atomic
  chrono
  container
  context
  coroutine
  date_time
  exception
  filesystem
  fiber
  graph
  iostreams
  locale
  log
  math
  nowide
  program_options
  random
  regex
  serialization
  stacktrace
  system
  thread
  timer
  type_erasure
  wave
)

# Optional libraries that require external dependencies:
option(BOOST_ENABLE_PYTHON       "Build Boost.Python if Python is available" OFF)
option(BOOST_ENABLE_MPI          "Build Boost.MPI if MPI is available"       OFF)
option(BOOST_ENABLE_TEST         "Build Boost.Test (unit test framework)"    OFF)

set(_BOOST_LIBS_OPTIONAL "")
if(BOOST_ENABLE_PYTHON)
  list(APPEND _BOOST_LIBS_OPTIONAL python)
endif()
if(BOOST_ENABLE_MPI)
  list(APPEND _BOOST_LIBS_OPTIONAL mpi)
  # graph_parallel depends on MPI as well; include when MPI is on:
  list(APPEND _BOOST_LIBS_OPTIONAL graph_parallel)
endif()
if(BOOST_ENABLE_TEST)
  list(APPEND _BOOST_LIBS_OPTIONAL test)
endif()

# Final list passed to the superproject:
set(_BOOST_LIBS ${_BOOST_LIBS_SAFE} ${_BOOST_LIBS_OPTIONAL})

# ========= ExternalProject definition =========
ExternalProject_Add(Boost_external
  GIT_REPOSITORY            ${_boost_git_url}
  GIT_TAG                   ${_boost_git_tag}
  GIT_SHALLOW               FALSE
  GIT_PROGRESS              TRUE

  UPDATE_COMMAND            ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git submodule update --init --recursive

  BUILD_IN_SOURCE           OFF

  # Configure Boost with CMake Superproject
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND}
      -S <SOURCE_DIR>
      -B <BINARY_DIR>
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=$<IF:$<CONFIG:>,${CMAKE_BUILD_TYPE},$<CONFIG>>  # support single & multi-config
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
      -DBUILD_TESTING=OFF
      # Install headers + requested libs; header-only libs are always usable
      -DBOOST_INSTALL_HEADERS=ON
      # IMPORTANT: pass the list as a single argument
      -DBOOST_INCLUDE_LIBRARIES:STRING=${_BOOST_LIBS}
      # Propagate toolchain/flags if you define them in the parent
      -DCMAKE_CXX_FLAGS=${boost_CXX_Flags}

  # Build / Install (multi-config generators honor ${CMAKE_CFG_INTDIR})
  BUILD_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${CMAKE_CFG_INTDIR}

  INSTALL_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --target install --config ${CMAKE_CFG_INTDIR}
)

# ========= Export properties to the superbuild =========
ExternalProject_Get_Property(Boost_external INSTALL_DIR)
ExternalProject_Get_Property(Boost_external SOURCE_DIR)

# Convenience prefix
set(SCI_BOOST_PREFIX "${INSTALL_DIR}")

# Help both Config and Module (FindBoost) modes in downstream projects:
#  - Boost_ROOT + CMAKE_PREFIX_PATH: lets find_package(Boost CONFIG) discover Boost-<ver> automatically.
#  - We'll still compute Boost_DIR explicitly for consumers that cache/use it directly.
set(Boost_ROOT "${SCI_BOOST_PREFIX}" CACHE PATH "Boost install prefix" FORCE)

# Make sure the prefix is searched (prefer prepending so our Boost wins over system)
if(NOT DEFINED CMAKE_PREFIX_PATH)
  set(CMAKE_PREFIX_PATH "")
endif()
list(PREPEND CMAKE_PREFIX_PATH "${SCI_BOOST_PREFIX}")
# Re-cache the modified prefix path for dependents in the same configure
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE PATH "Prefix search path" FORCE)

# Compute Boost_DIR robustly (handles versioned 'Boost-<ver>' and rare unversioned 'Boost/')
set(_boost_cmake_root "${SCI_BOOST_PREFIX}/lib/cmake")
set(_boost_dir_guess "")

if(EXISTS "${_boost_cmake_root}")
  # Prefer versioned directories (Boost-1.90.0, Boost-1.91.0, ...)
  file(GLOB _boost_cfg_candidates "${_boost_cmake_root}/Boost-*")
  foreach(_cand IN LISTS _boost_cfg_candidates)
    if(EXISTS "${_cand}/BoostConfig.cmake")
      set(_boost_dir_guess "${_cand}")
      break()
    endif()
  endforeach()

  # Fallback: some older packs use unversioned 'Boost/'
  if(NOT _boost_dir_guess AND EXISTS "${_boost_cmake_root}/Boost/BoostConfig.cmake")
    set(_boost_dir_guess "${_boost_cmake_root}/Boost")
  endif()
endif()

# Expose Boost_DIR for consumers that look it up directly
# (It may not exist on the first configure until after install; that's okay.)
set(Boost_DIR "${_boost_dir_guess}" CACHE PATH "Boost package directory (for find_package Boost)" FORCE)

# Fallback hints for Module mode (FindBoost.cmake)
set(SCI_BOOST_INCLUDE "${SCI_BOOST_PREFIX}/include" CACHE PATH "Boost include directory" FORCE)

# Handle lib vs lib64 on Windows and some Linux distros
if (EXISTS "${SCI_BOOST_PREFIX}/lib64")
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib64")
else()
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib")
endif()
set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_LIBRARY_DIR}" CACHE PATH "Boost library directory" FORCE)

# Optional: legacy use file (kept only if your build still includes it)
set(SUPERBUILD_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(SCI_BOOST_USE_FILE "${SCI_BOOST_PREFIX}/UseBoost.cmake")
if (EXISTS "${SUPERBUILD_DIR}/UseBoost.cmake")
  configure_file(${SUPERBUILD_DIR}/UseBoost.cmake ${SCI_BOOST_USE_FILE} COPYONLY)
endif()

# Diagnostics (helpful during first passes)
message(STATUS "[Boost_ext] INSTALL_DIR: ${SCI_BOOST_PREFIX}")
message(STATUS "[Boost_ext] Boost_ROOT:  ${Boost_ROOT}")
message(STATUS "[Boost_ext] Boost_DIR:   ${Boost_DIR}")
message(STATUS "[Boost_ext] Include dir: ${SCI_BOOST_INCLUDE}")
message(STATUS "[Boost_ext] Lib dir:     ${SCI_BOOST_LIBRARY_DIR}")
message(STATUS "[Boost_ext] BOOST_INCLUDE_LIBRARIES: ${_BOOST_LIBS}")

if (EXISTS "${Boost_DIR}/BoostConfig.cmake")
  message(STATUS "[Boost_ext] Found BoostConfig.cmake at: ${Boost_DIR}/BoostConfig.cmake")
else()
  message(STATUS "[Boost_ext] (Will be created after Boost install) Expected under: ${_boost_cmake_root}")
endif()
