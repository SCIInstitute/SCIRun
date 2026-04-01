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

#
# Boost External Project for SCIRun Superbuild
#

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# ------------------------------------------------------------------------------
# Boost compile flags
# ------------------------------------------------------------------------------
SET(boost_CXX_Flags "-DBOOST_ALL_NO_LIB=1")
IF(APPLE)
  LIST(APPEND boost_CXX_Flags "-DBOOST_LCAST_NO_WCHAR_T" "-DBOOST_THREAD_DONT_USE_ATOMIC")
ENDIF()
IF(WIN32)
  LIST(APPEND boost_CXX_Flags "-DBOOST_BIND_ENABLE_STDCALL")
ENDIF()

SET(boost_DEPENDENCIES)

# ------------------------------------------------------------------------------
# Explicit library list
# ------------------------------------------------------------------------------
SET(boost_Libraries
  atomic
  chrono
  date_time
  exception
  filesystem
  program_options
  regex
  serialization
  thread
  CACHE INTERNAL "Boost library names."
)

IF(BUILD_WITH_PYTHON)
  ADD_DEFINITIONS(-DBOOST_PYTHON_STATIC_LIB=1)
  LIST(APPEND boost_Libraries python)
  LIST(APPEND boost_DEPENDENCIES Python_external)
  LIST(APPEND boost_CXX_Flags "-DBOOST_PYTHON_STATIC_MODULE" "-DBOOST_PYTHON_STATIC_LIB")
ENDIF()

# ------------------------------------------------------------------------------
# Darwin/Unix/Windows consistency flags
# ------------------------------------------------------------------------------
IF(UNIX)
  ADD_DEFINITIONS(-DBOOST_NO_CXX11_ALLOCATOR)
ENDIF()

IF(WIN32)
  SET(FORCE_64BIT_BUILD ON)
ENDIF()

SET(_boost_git_url "https://github.com/CIBC-Internal/boost.git")
SET(_boost_git_tag "v1.90.0")

# ------------------------------------------------------------------------------
# Compute Python library filename
# ------------------------------------------------------------------------------
IF(WIN32)
  SET(SCI_PYTHON_LIBRARY_FILE
      ${SCI_PYTHON_LIBRARY_DIR}/${SCI_PYTHON_NAME}.lib)
ELSEIF(APPLE)
  SET(SCI_PYTHON_LIBRARY_FILE
      ${SCI_PYTHON_LIBRARY_DIR}/libpython${SCI_PYTHON_VERSION_SHORT}.dylib)
ELSE()
  SET(SCI_PYTHON_LIBRARY_FILE
      ${SCI_PYTHON_LIBRARY_DIR}/libpython${SCI_PYTHON_VERSION_SHORT}.so)
ENDIF()

# ------------------------------------------------------------------------------
# Compute b2 Python flags (MUST be separate arguments)
# ------------------------------------------------------------------------------
IF(BUILD_WITH_PYTHON)
  SET(BOOST_PYTHON_WITH_FLAG  --with-python)
  #SET(BOOST_PYTHON_EXE_FLAG   python=${SCI_PYTHON_EXE})
  #SET(BOOST_PYTHON_INC_FLAG   include=${SCI_PYTHON_INCLUDE})
  #SET(BOOST_PYTHON_LIB_FLAG   library-path=${SCI_PYTHON_LIBRARY_DIR})
  SET(BOOST_PYTHON_VERSION_FLAG python-version=${SCI_PYTHON_VERSION_SHORT})
ELSE()
  SET(BOOST_PYTHON_WITH_FLAG  --without-python)
  SET(BOOST_PYTHON_EXE_FLAG   "")
  SET(BOOST_PYTHON_INC_FLAG   "")
  SET(BOOST_PYTHON_LIB_FLAG   "")
ENDIF()

if(WIN32)
  # Convert Windows PATH to CMake-friendly forward-slash form
  file(TO_CMAKE_PATH "$ENV{PATH}" _SCIRUN_ENV_PATH_CMAKE)
else()
  set(_SCIRUN_ENV_PATH_CMAKE "$ENV{PATH}")
endif()
# ------------------------------------------------------------------------------
# ExternalProject_Add: Boost
# ------------------------------------------------------------------------------
ExternalProject_Add(Boost_external
  DEPENDS ${boost_DEPENDENCIES}
  GIT_REPOSITORY ${_boost_git_url}
  GIT_TAG ${_boost_git_tag}
  BUILD_IN_SOURCE ON
  PATCH_COMMAND ""
  INSTALL_COMMAND ""

  CMAKE_CACHE_ARGS
      -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DFORCE_64BIT_BUILD:BOOL=${FORCE_64BIT_BUILD}

      # ---------- Python ON/OFF controlled by SCIRun option ----------
      -DBUILD_PYTHON:BOOL=${BUILD_WITH_PYTHON}
      #-DBOOST_ENABLE_PYTHON:BOOL=${BUILD_WITH_PYTHON}

      # ---------- FORCE FindPython3 to use SCIRun Python, not Xcode ----------
      -DPython3_FIND_FRAMEWORK:STRING=NEVER
      -DPython3_FIND_STRATEGY:STRING=LOCATION
      -DPython3_FIND_REQUIRED:BOOL=${BUILD_WITH_PYTHON}
      -DPython3_ROOT_DIR:PATH=${SCI_PYTHON_ROOT_DIR}
      -DPython3_EXECUTABLE:FILEPATH=${SCI_PYTHON_EXE}
      -DPython3_INCLUDE_DIR:PATH=${SCI_PYTHON_INCLUDE}
      -DPython3_LIBRARY:FILEPATH=${SCI_PYTHON_LIBRARY_FILE}

  CMAKE_COMMAND_ENV
      "PYTHONHOME=${SCI_PYTHON_ROOT_DIR}"
      "PYTHONPATH="
      "PATH=${SCI_PYTHON_ROOT_DIR}/bin:${_SCIRUN_ENV_PATH_CMAKE}"
      "CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=FALSE"
      "CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH=FALSE"
      "CMAKE_FIND_FRAMEWORK=NEVER"
      "Python3_FIND_FRAMEWORK=NEVER"
      "Python3_FIND_STRATEGY=LOCATION"
)

# ------------------------------------------------------------------------------
# Internal paths from ExternalProject
# ------------------------------------------------------------------------------
ExternalProject_Get_Property(Boost_external INSTALL_DIR)
ExternalProject_Get_Property(Boost_external SOURCE_DIR)

IF(WIN32)
  SET(_B2_CMD ${SOURCE_DIR}/b2.exe)
  SET(_B2_BOOTSTRAP_CMD bootstrap.bat)
ELSE()
  SET(_B2_CMD ${SOURCE_DIR}/b2)
  SET(_B2_BOOTSTRAP_CMD ./bootstrap.sh)
ENDIF()

# --------------------------------------------------------------
# Step: bootstrap b2
# --------------------------------------------------------------
ExternalProject_Add_Step(Boost_external bootstrap_b2
  COMMAND ${_B2_BOOTSTRAP_CMD}
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES update
  INDEPENDENT 1
  COMMENT "Bootstrapping b2"
)

# --------------------------------------------------------------
# Step: write project-config.jam (AFTER bootstrap)
# --------------------------------------------------------------
ExternalProject_Add_Step(Boost_external write_project_config
  COMMAND ${CMAKE_COMMAND}
      -DOUTPUT_FILE=${SOURCE_DIR}/project-config.jam
      -DVERSION=${SCI_PYTHON_VERSION_SHORT}
      -DEXE=${SCI_PYTHON_EXE}
      -DINCLUDE=${SCI_PYTHON_INCLUDE}
      -DLIBDIR=${SCI_PYTHON_LIBRARY_DIR}
      -P ${SUPERBUILD_DIR}/WriteProjectConfigJam.cmake

  DEPENDEES bootstrap_b2
  INDEPENDENT 1

  COMMENT "Overwriting project-config.jam with Python toolset AFTER bootstrap"
)

# --------------------------------------------------------------
# Step: verify project-config.jam
# --------------------------------------------------------------
ExternalProject_Add_Step(Boost_external verify_project_config
  COMMAND ${CMAKE_COMMAND} -E echo "==== VERIFY project-config.jam AFTER bootstrap ===="
  COMMAND ${CMAKE_COMMAND} -E cat ${SOURCE_DIR}/project-config.jam

  DEPENDEES write_project_config
  INDEPENDENT 1

  COMMENT "Verifying final project-config.jam"
)

# ------------------------------------------------------------------------------
# Step: b2 headers
# ------------------------------------------------------------------------------
ExternalProject_Add_Step(Boost_external stage_headers
  COMMAND ${_B2_CMD} headers
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES write_project_config
  COMMENT "Running b2 headers"
)

# ------------------------------------------------------------------------------
# Step: Install full headers
# ------------------------------------------------------------------------------
ExternalProject_Add_Step(Boost_external install_full_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/include
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${INSTALL_DIR}/include/boost
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/boost ${INSTALL_DIR}/include/boost
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES stage_headers
  DEPENDERS install
  COMMENT "Installing full Boost headers"
)

# ------------------------------------------------------------------------------
# Step: Build Boost libraries
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------
# Common Boost b2 build arguments (used for echo + real build)
# ------------------------------------------------------------------
if(WIN32)
  set(_BOOST_CXXFLAGS "")
else()
  set(_BOOST_CXXFLAGS cxxflags=-fPIC)
endif()

set(_BOOST_B2_ARGS
  --with-atomic
  --with-chrono
  --with-date_time
  --with-filesystem
  --with-program_options
  --with-regex
  --with-serialization
  --with-thread

  ${BOOST_PYTHON_WITH_FLAG}
  ${BOOST_PYTHON_EXTRA_FLAGS}

  link=static
  runtime-link=static
  variant=release,debug
  threading=multi
  stage
)

ExternalProject_Add_Step(Boost_external build_libs
  COMMAND ${CMAKE_COMMAND} -E echo "=== B2 PYTHON FLAGS ==="
  COMMAND ${CMAKE_COMMAND} -E echo "${BOOST_PYTHON_WITH_FLAG}"
  COMMAND ${CMAKE_COMMAND} -E echo "${BOOST_PYTHON_EXE_FLAG}"
  COMMAND ${CMAKE_COMMAND} -E echo "${BOOST_PYTHON_INC_FLAG}"
  COMMAND ${CMAKE_COMMAND} -E echo "${BOOST_PYTHON_LIB_FLAG}"
  COMMAND ${CMAKE_COMMAND} -E echo "SCI_PYTHON_EXE = ${SCI_PYTHON_EXE}"
  COMMAND ${CMAKE_COMMAND} -E echo "SCI_PYTHON_INCLUDE = ${SCI_PYTHON_INCLUDE}"
  COMMAND ${CMAKE_COMMAND} -E echo "SCI_PYTHON_LIBRARY_DIR = ${SCI_PYTHON_LIBRARY_DIR}"

  COMMAND ${CMAKE_COMMAND} -E echo "=== B2 FULL CMD ==="
  COMMAND ${CMAKE_COMMAND} -E echo "${_B2_CMD} ${_BOOST_B2_ARGS}"

  COMMAND ${_B2_CMD} ${_BOOST_B2_ARGS}

  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES stage_headers
  COMMENT "Building Boost static libraries (Debug + Release)"
)

# ------------------------------------------------------------------------------
# Export Boost library info
# ------------------------------------------------------------------------------
SET(SCI_BOOST_INCLUDE ${SOURCE_DIR})
SET(SCI_BOOST_LIBRARY_DIR ${SOURCE_DIR}/stage/lib)
#SET(SCI_BOOST_USE_FILE ${INSTALL_DIR}/UseBoost.cmake)

SET(BOOST_PREFIX "boost_")
SET(THREAD_POSTFIX "")

#SET(SCI_BOOST_LIBRARY)
#FOREACH(lib ${boost_Libraries})
#  IF(lib STREQUAL "python")
#    # Python library is versioned: e.g., boost_python313
#    LIST(APPEND SCI_BOOST_LIBRARY "${BOOST_PREFIX}${lib}${SCI_PYTHON_VERSION_SHORT_WIN32}")
#  ELSE()
#    LIST(APPEND SCI_BOOST_LIBRARY "${BOOST_PREFIX}${lib}${THREAD_POSTFIX}")
#  ENDIF()
#ENDFOREACH()

CONFIGURE_FILE(${SUPERBUILD_DIR}/BoostConfig.cmake.in
               ${INSTALL_DIR}/BoostConfig.cmake @ONLY)
#CONFIGURE_FILE(${SUPERBUILD_DIR}/UseBoost.cmake
#               ${SCI_BOOST_USE_FILE} COPYONLY)

SET(Boost_DIR ${INSTALL_DIR} CACHE PATH "")
MESSAGE(STATUS "Boost_DIR: ${Boost_DIR}")