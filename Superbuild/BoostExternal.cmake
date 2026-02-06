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
  # Add here only if/when linker complains:
  # chrono; thread; regex; program_options; iostreams; date_time; serialization
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

# ========= ExternalProject definition =========
ExternalProject_Add(Boost_external
  GIT_REPOSITORY            ${_boost_git_url}
  GIT_TAG                   ${_boost_git_tag}
  GIT_SHALLOW               FALSE
  GIT_PROGRESS              TRUE

  # Ensure submodules are available
  UPDATE_COMMAND            ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git submodule update --init --recursive

  BUILD_IN_SOURCE           OFF

  # Configure Boost with CMake Superproject (build only compiled libs)
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND}
      -S <SOURCE_DIR>
      -B <BINARY_DIR>
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=$<IF:$<CONFIG:>,${CMAKE_BUILD_TYPE},$<CONFIG>>  # single & multi-config
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
      -DBUILD_TESTING=OFF
      -DBOOST_INSTALL_HEADERS=ON
      -DBOOST_INSTALL_LAYOUT=system
      -DBOOST_INCLUDE_LIBRARIES:STRING=${_BOOST_LIBS}   # compiled libs only
      -DCMAKE_CXX_FLAGS=${boost_CXX_Flags}

  BUILD_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${CMAKE_CFG_INTDIR}

  INSTALL_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --target install --config ${CMAKE_CFG_INTDIR}
)

# ========= Cross-platform b2 header staging =========
# 1) Bootstrap b2
if(WIN32)
  set(_B2_BOOTSTRAP_CMD cmd /c bootstrap.bat)
  set(_B2_HEADERS_CMD   cmd /c .\\b2 headers)
else()
  # Linux/macOS
  set(_B2_BOOTSTRAP_CMD ./bootstrap.sh)
  set(_B2_HEADERS_CMD   ./b2 headers)
endif()

ExternalProject_Add_Step(Boost_external bootstrap_b2
  COMMAND ${_B2_BOOTSTRAP_CMD}
  WORKING_DIRECTORY <SOURCE_DIR>
  DEPENDEES update
  DEPENDERS configure      # ensure b2 exists before configure/build
  COMMENT "Bootstrapping Boost.Build (b2)"
)

# 2) Generate the full 'boost/' header tree
ExternalProject_Add_Step(Boost_external stage_headers
  COMMAND ${_B2_HEADERS_CMD}
  WORKING_DIRECTORY <SOURCE_DIR>
  DEPENDEES bootstrap_b2
  DEPENDERS install        # must happen before/with install
  COMMENT "Running 'b2 headers' to generate the boost/ header tree"
)

# 3) Copy the entire header tree into the install prefix
ExternalProject_Add_Step(Boost_external install_full_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
  COMMAND ${CMAKE_COMMAND} -E remove_directory <INSTALL_DIR>/include/boost
  COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/boost <INSTALL_DIR>/include/boost
  DEPENDEES stage_headers
  DEPENDERS install
  COMMENT "Copying full Boost headers to <INSTALL_DIR>/include/boost"
)

# ========= Export properties for downstream =========
ExternalProject_Get_Property(Boost_external INSTALL_DIR)
ExternalProject_Get_Property(Boost_external SOURCE_DIR)

# Convenience prefix
set(SCI_BOOST_PREFIX "${INSTALL_DIR}")

# For both Config and Module find modes
set(Boost_ROOT "${SCI_BOOST_PREFIX}" CACHE PATH "Boost install prefix" FORCE)

# Make our Boost take precedence over system
if(NOT DEFINED CMAKE_PREFIX_PATH)
  set(CMAKE_PREFIX_PATH "")
endif()
list(PREPEND CMAKE_PREFIX_PATH "${SCI_BOOST_PREFIX}")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE PATH "Prefix search path" FORCE)

# Compute Boost_DIR robustly (versioned Boost-<ver> or fallback Boost/)
set(_boost_cmake_root "${SCI_BOOST_PREFIX}/lib/cmake")
set(_boost_dir_guess "")

if(EXISTS "${_boost_cmake_root}")
  file(GLOB _boost_cfg_candidates "${_boost_cmake_root}/Boost-*")
  foreach(_cand IN LISTS _boost_cfg_candidates)
    if(EXISTS "${_cand}/BoostConfig.cmake")
      set(_boost_dir_guess "${_cand}")
      break()
    endif()
  endforeach()
  if(NOT _boost_dir_guess AND EXISTS "${_boost_cmake_root}/Boost/BoostConfig.cmake")
    set(_boost_dir_guess "${_boost_cmake_root}/Boost")
  endif()
endif()

set(Boost_DIR "${_boost_dir_guess}" CACHE PATH "Boost package directory (for find_package Boost)" FORCE)

# Concrete include/lib paths (SCIRun consumes these)
set(SCI_BOOST_INCLUDE "${SCI_BOOST_PREFIX}/include" CACHE PATH "Boost include directory" FORCE)
if (EXISTS "${SCI_BOOST_PREFIX}/lib64")
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib64")
else()
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_PREFIX}/lib")
endif()
set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_LIBRARY_DIR}" CACHE PATH "Boost library directory" FORCE)

# Optional: legacy use file
set(SUPERBUILD_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(SCI_BOOST_USE_FILE "${SCI_BOOST_PREFIX}/UseBoost.cmake")
if (EXISTS "${SUPERBUILD_DIR}/UseBoost.cmake")
  configure_file(${SUPERBUILD_DIR}/UseBoost.cmake ${SCI_BOOST_USE_FILE} COPYONLY)
endif()

# Diagnostics
message(STATUS "[Boost_ext] INSTALL_DIR: ${SCI_BOOST_PREFIX}")
message(STATUS "[Boost_ext] Boost_ROOT:  ${Boost_ROOT}")
message(STATUS "[Boost_ext] Boost_DIR:   ${Boost_DIR}")
message(STATUS "[Boost_ext] Include dir: ${SCI_BOOST_INCLUDE}")
message(STATUS "[Boost_ext] Lib dir:     ${SCI_BOOST_LIBRARY_DIR}")
message(STATUS "[Boost_ext] BOOST_INCLUDE_LIBRARIES (compiled only): ${_BOOST_LIBS}")

if (EXISTS "${Boost_DIR}/BoostConfig.cmake")
  message(STATUS "[Boost_ext] Found BoostConfig.cmake at: ${Boost_DIR}/BoostConfig.cmake")
else()
  message(STATUS "[Boost_ext] (Will be created after Boost install) Expected under: ${_boost_cmake_root}")
endif()